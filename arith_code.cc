/*
 * @author Andrea C. G. Mennucci
 *
 * @file arith_code.cc
 *
 * @copyright (C) 2010-2019 Andrea C. G. Mennucci
 *
 * The new BSD License is applied to this software, see LICENSE.txt
 *
 */


#include "stdio.h"
#include <stdlib.h>
#include <stdint.h>
#include "assert.h"
#include "math.h"
# include <string>
#include <functional>

namespace AC {

/******  tunable definitions  **********/

//! value that represents a non symbol
const int NO_SYMBOL = -1;

//! value that represents a succeful deflushing
//! (note that this symbol is not counted in the internal count of symbols)
const int FLUSH_SYMBOL = -2;

//! minimum value for a symbol,
//! 0 is the common choice for computer programs,
//! 1 is the common choice for people
const int MIN_SYMBOL = 0;

// enable this to check that, in decoder, fast search and slow search always agree
// (the code will be much slower!)
// #define AC_CHECK_FAST_SEARCH

#ifndef  AC_representation_bitsize
//! the number of bits to represent the intervals
#define AC_representation_bitsize 31
#endif

/* enable central zoom */
#define AC_QUARTER_ZOOM


/*********** general constants ****/

/*! types for variables defining intervals */
#if (AC_representation_bitsize <= 15)
typedef uint16_t    I_t;
typedef uint32_t  long_I_t;
#define AC_SIZE 16
#elif (AC_representation_bitsize <= 31)
typedef uint32_t    I_t;
typedef uint64_t  long_I_t;
#define AC_SIZE 32
#elif (AC_representation_bitsize <= 63)
typedef uint64_t        I_t;
typedef __int128  long_I_t;
#define AC_SIZE 64
#else
#error "ARITHMETIC CODEC says: too many bits"
#endif

/*! types for variables defining frequencies */
typedef  I_t F_t;
/*! the sum of all frequencies of symbols cannot exceed this value */
const  I_t  MAX_FREQ = ((I_t)1)      << (AC_representation_bitsize-2) ;

//! struct to represent an interval
typedef  std::pair<I_t, I_t>  interval_t;

//! type for callbacks
typedef std::function<void(int,void *)> callback_t;


const char *bit_rep[16] = {
    [ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
    [ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
    [ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
    [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
};

const char *hex_rep[16] = {
    [ 0] = "0", [ 1] = "1", [ 2] = "2", [ 3] = "3",
    [ 4] = "4", [ 5] = "5", [ 6] = "6", [ 7] = "7",
    [ 8] = "8", [ 9] = "9", [10] = "A", [11] = "B",
    [12] = "C", [13] = "D", [14] = "E", [15] = "F",
};


//! prints an integer in a string
std::string string_binary(I_t b)
{
  int s=AC_SIZE;
  std::string st;
  while(s>0) {
    st =  hex_rep[b & 0x0F] + st;
    b = b >> 4;
    s=s-4;
    if(s && (0==(s & 15)))
      st = "." + st;
  }
  return st;
}



/***********  stampa operazioni eseguite **************/

#ifdef AC_VERBOSE

#define printINTERVALextra(L,H) \
 printf("%6s %5ld % .4f %6s %5ld % .4f b %d s %d z %d" ,\
    __STRING(L),(long)L, ASFLOAT(L),\
    __STRING(H), (long)H,ASFLOAT(H+1),n_bits,n_symbs,n_zooms);

#define printINTERVAL_OLD(L,H) \
 printf("%6s % .6f %6s % .6f , b %d s %d z %d o %d" ,\
    __STRING(L), ASFLOAT(L),\
	__STRING(H), ASFLOAT(H+1),n_bits,n_symbs,n_zooms,n_outputs);

#define printINTERVAL(L,H)				\
 printf("%6s %s %6s %s ",\
	__STRING(L), string_binary(L).c_str(),			\
	__STRING(H), string_binary(H).c_str());


#define printHL() printINTERVAL(Slow,Shigh)

#define printBHBL() printINTERVAL(Blow,Bhigh)

#define PRINT(A,args...) printf(prefix); printf(" : "); printf(A,##args);

#define P(A) printf(prefix); printf(" : ");printHL();printf(": when " A "\n");
#define PB(A) printf(prefix); printf(" : ");printBHBL();\
  printf(" significant %d : when " A "\n",significant_bits);

#else
#define P(A)
#define PB(A)
#define PRINT(A...)
#endif

void freq2cum_freq(F_t cum_freq[], F_t freq[], int max_symb, int  assert_non_zero);


/* base class ; should not be used; contains the logic
 * common to encoder and decoder
 */
class Base{

protected:
  //typedef std::function<void(int,Base *)> callback_B_t;


#ifdef AC_QUARTER_ZOOM
  /* counts virtual bits, in case of centered zooms */
  unsigned int bitsToFollow;
  /* stores the value of the virtual bit , or -1 if there is no virtual bit */
  int virtual_bit;
#endif

  const I_t One = 1;
  //! representation of 1
  const I_t  Top =  (One   << AC_representation_bitsize);
  //! representation of 1/4
  const I_t  Qtr =  (One   << (AC_representation_bitsize-2));
  //! representation of point preceding 1/4
  const I_t  QtrMinus =  (One   << (AC_representation_bitsize-2)) - One;
  //! Representation of 3/4
  const I_t  Half = (Qtr*2);
  //! representation of 3/4
  const I_t  ThreeQtr = (Qtr*3);

  /*! how many symbols in the  special cumulative table used for flushing */
  static const int n_symbols_flush = 3;
  /*! special cumulative table used for flushing */
  F_t cum_freq_flush[n_symbols_flush+1] =  { Qtr , QtrMinus , 1 , 0 };


  //! S-interval left extreme
  I_t  Slow;
  //! S-interval right extreme (included in the interval)
  I_t  Shigh;
  //! S-interval width
  I_t Srange;

  //! B-interval left extreme
  I_t  Blow;
  //! B-interval right extreme
  I_t Bhigh;

  // operations on intervals
  void  doubleit()  { Slow = 2*Slow; Shigh = 2*Shigh+1; Srange=Shigh-Slow+1; assert(Srange>0); Blow=Blow*2; Bhigh = 2*Bhigh+1; }
  void  doublehi()  { Slow -= Half; Shigh -= Half; Blow -= Half; Bhigh -= Half;  doubleit();  P("doublehi"); }
  void  doublelow() { doubleit(); P("doublelow"); }
  void  doublecen() { Slow -= Qtr; Shigh -= Qtr; Blow -= Qtr;Bhigh -= Qtr;  doubleit(); P("doublecen"); }

  //! name of the class, for printing
  const char *prefix;

  //! significant bits (used in the decoder)
  unsigned int significant_bits;

  //! number of bits inserted in the state
  unsigned int n_in_bits,
  //! number of symbols inserted in the state
    n_in_symbs,
  //! number of zooms
    n_zooms,
  //! number of bits extracted from the state
    n_out_bits,
  //! number of symbols extracted from the state
    n_out_symbs;

  Base() {
    n_in_bits=0; n_in_symbs=0; n_zooms=0; n_out_bits=0,  n_out_symbs=0;
    Blow = Slow = 0; Bhigh = Shigh=(I_t)(Top-1); Srange=Shigh-Slow+1;
    significant_bits=0;
#ifdef AC_QUARTER_ZOOM
    bitsToFollow=0;  virtual_bit=-1;
#endif
    prefix="base";
  };

  //! a pointer to data that the callbacks receive as second argument
  //! it is initialized with a pointer to the class
  void *callback_data = NULL;
public:
  //! a pointer to user data, that the callbacks may then use
  void *payload = NULL;

    //! number of bits inserted in the state
  unsigned int number_input_bits() { return n_in_bits ;};
  //! number of symbols inserted in the state
  unsigned int number_input_symbols() { return n_in_symbs;};
  //! number of zooms
  //  n_zooms,
  //! number of bits extracted from the state
  unsigned int number_output_bits() { return n_out_bits;};
  //! number of symbols extracted from the state
  unsigned int number_output_symbols() { return n_out_symbs;};

  //! print the internal state
  void print_state()
  {
    printf("%s : Blo %s Bhi %s (significant %d)\n          Slo %s Shi %s\n         IN bits %d symb %d zooms %d OUT bits %d symbs %d\n" ,
	   prefix, string_binary(Blow).c_str(), string_binary(Bhigh).c_str(),significant_bits,
	   string_binary(Slow).c_str(), string_binary(Shigh).c_str(),
	   n_in_bits,n_in_symbs,n_zooms,n_out_bits,n_out_symbs);
#ifdef AC_QUARTER_ZOOM
    printf("   bits to follow  %d virtual bit %d\n" ,
	   bitsToFollow, virtual_bit);
#endif
  }

  //! return the S_interval
  interval_t  S_interval()
  {   interval_t i= {Slow, Shigh}; return i ; };

  //! return the B_interval
  interval_t  B_interval()
  {   interval_t i= {Blow, Bhigh}; return i ; };

private:
  /*! returns 0 , 1 or -1 if no bit can be pulled at this moment ; resize S-interval and B-interval accordingly */
  int  resize_pull_one_bit()
  {
    if ( Shigh < Half) {
      doublelow();
      significant_bits--;
      n_zooms++;
      return 0;
    } else if (Slow>=Half) {
      doublehi();
      significant_bits--;
      n_zooms++;
      return 1;
    }
#ifdef AC_QUARTER_ZOOM
    else while ( Slow >=Qtr && Shigh < ThreeQtr) {
	/* add a  virtual bit */
	bitsToFollow += 1;
      	doublecen();
	PRINT("virtual bitsToFollow %d\n", bitsToFollow);
	significant_bits--;
	n_zooms++;
    }
#endif
    // at this point the width of the S-interval is more than 1/4
    // otherwise we would have returned a bit, or doublecen() many times
    assert(Srange > Qtr);
    return -1;
  };

public:

  //! outputs one bit from the state, if available, else -1
  int output_bit()
  {
#ifdef AC_QUARTER_ZOOM
    // are there virtual bits
    if(virtual_bit >= 0 &&  bitsToFollow > 0) {
      bitsToFollow--;
      n_out_bits++;
      PRINT(" pull virtual bit %d, bits_to_follow %d\n", virtual_bit, bitsToFollow);
      int v = virtual_bit;
      if ( bitsToFollow <= 0 )
	// after this last virtual bit we will output a real bit
	 virtual_bit = -1;
      return v;
    }
#endif
    int b = resize_pull_one_bit();
#ifdef AC_QUARTER_ZOOM
    // queue virtual bits
    if ( b >= 0 && bitsToFollow > 0)
      // after this bit we will output all virtual bits
      virtual_bit = 1 - b;
#endif
    if(b>= 0)
      n_out_bits++;
    PRINT(" pull bit %d  (virtual bit  %d, bitsToFollow %d ) \n", b, virtual_bit, bitsToFollow);
    return b;
  }

  /*!  outputs multiple bits, returns them using a callback (if not null; else they are lost) */
  void output_bits(callback_t out)
  {
    int b;
    while ( -1 != (b=resize_pull_one_bit()) ) {
      // is not  incremented in resize_pull_one_bit
      n_out_bits++;
      PRINT(" output bit %d\n", b );
      if (out) out(b, callback_data);
#ifdef AC_QUARTER_ZOOM
      virtual_bit = 1 - b;
      while (0 < bitsToFollow) {
	n_out_bits++;
	bitsToFollow--;
	PRINT(" output virtual bit %d, bits_to_follow %d\n", virtual_bit, bitsToFollow);
	if (out) out(virtual_bit, callback_data);
      }
      virtual_bit = -1;
#endif
    }
  };

private:
  /*! divides Slow - Shigh in subintervals : returns the beginning of each interval;
     note that intervals are in reverse order wrt symbols, that is, 
     symb=0 gives the rightmost subinterval,
     symb=max_symbols-1 is the leftmost */
  I_t separ_low_high(int symb, I_t cum_freq[])
  {
    assert( symb >= MIN_SYMBOL );
    //long_I_t Srange=(long_I_t)high-low+1;
    /* the following may OVERFLOW or UNDERFLOW */
    return  Slow + (I_t) ( ( ((long_I_t)Srange) *  ((long_I_t)cum_freq[symb]) ) / ((long_I_t)cum_freq[0]) );
  }
protected:
  /*! right extreme of a S-sub-interval ; note that symbols start from 0 here */
  I_t interval_right(int symb, I_t cum_freq[]) {
    return separ_low_high(symb,cum_freq)-1;
  };
  /*! left extreme of a S-sub-interval */
  I_t interval_left(int symb, I_t cum_freq[]) {
    return separ_low_high(symb+1,cum_freq);
  };

  /*! put symbol in S-interval by splitting it and choosing a subinterval, proportional to the frequencies */
  void push_symbol(int symb, I_t cum_freq[])
  {
    I_t l,h;
    PRINT(" put symb %d in S-interval\n",symb);

    n_in_symbs++;
    h =  interval_right(symb,cum_freq);
    l =  interval_left (symb,cum_freq);
    Shigh = h;
    Slow = l;
    Srange = Shigh - Slow + 1;
    P("after push symbol");
    if(Slow>Shigh)
      {printf(" ************* S-interval underflow ***********\n");  print_state(); abort();}
  }

  /*! put bit in B-interval */
  void push_bit(int bit)
  {
    PRINT(" put bit %d in B-interval\n",bit);
    significant_bits++;
    n_in_bits++;
    if(bit==0)
      Bhigh=(Bhigh+1+Blow)/2-1;
    else
      Blow=(Bhigh+1+Blow)/2;
    PB("after push bit");
    /* check for underflow */
    if(Blow>Bhigh)
      {printf(" ************* B-interval underflow ************\n");   print_state(); abort();}
  };

public:
  ////// convenient functions to manage frequencies
  /*! if the output callback is used in decoding, then the "cumulative_frequencies"
   *    and "max_symbol" must be updated after each symbol
   *    is decoded, with the correct frequencies for the next symbol
   */
  F_t * cumulative_frequencies  = NULL;
  /*! if the output callback is used in decoding, then the "cumulative_frequencies"
   *    and "max_symbol" must be updated after each symbol
   *    is decoded, with the correct frequencies for the next symbol
   */
  I_t max_symbol = -1;

  /*! if the output callback is used in decoding, then the "cumulative_frequencies"
   *    and "max_symbol" must be used; these may be derived from
        a table of frequencies, that may be stored here
   */
  F_t * frequencies  = NULL;

  /*! if the output callback is used in decoding, then the "cumulative_frequencies"
   *    and "max_symbol" must be used;  "cumulative_frequencies"
       may be updated from "frequencies" using this call, that wraps freq2cum_freq()
   */
  void frequencies2cumulative_frequencies()
  {
    assert(cumulative_frequencies &&  frequencies &&  max_symbol > 0);
    freq2cum_freq( cumulative_frequencies, frequencies,  max_symbol, 1);
  }

};



/**************** ENCODER **********/
class Encoder : public Base { 

public:
  //  a good idea that is not working
  //typedef std::function<void(int,Encoder *)> callback_E_t;

  /*! callback when the encoder encodes a symbo */
  callback_t output_callback;

  /*! initialize, with a callback function that will output bits */
  Encoder(//! callback that will receive the encoded bits
	  callback_t output_callback_ = NULL)
  { prefix="encoder";
    output_callback = output_callback_;
    P("init"); PB("init");
    callback_data = this;
  }

  /*! insert a symbol; if output_callback() was provided, send it all available bits */
  void input_symbol( //! symbol to add to the state!
		    int symb,
		    //! cumulative frequencies (if not provided, use the internally stored ones
		    I_t cum_freq[] = NULL )
  {
    if (cum_freq == NULL) {
      cum_freq = cumulative_frequencies;
      assert(symb-MIN_SYMBOL < (int)max_symbol);
    }
    assert( symb >= MIN_SYMBOL );
    push_symbol(symb,cum_freq);
    if(output_callback) output_bits(output_callback);
  }

  //! flush the encoder
  //! (if you are not using callbacks, be sure to pull all bits out of the encoder
  //! before and after flushing, or underflow may occour)
  void flush()
  {
    PRINT(" start flushing\n");
    assert( Shigh >= Half && Half >= Slow );
    if ( (Shigh - Half)  < (Half - Slow) ) {
      input_symbol(2+MIN_SYMBOL, cum_freq_flush);
    } else {
      input_symbol(0+MIN_SYMBOL, cum_freq_flush);
    }
  }
};


/********************* DECODER **********/
class Decoder : public Base {
  //  a good idea that is not working
  //typedef std::function<void(int,Decoder *)> callback_D_t;

  /* callback when the decoder decodes a symbol */
  callback_t output_callback;


  /* when this callback is called, the  S-interval in the decoder is the same as the
   * S-interval in the encoder (at the same bitcount)
   * This is used only for 
   */
  callback_t bit_callback;

private:
  /* signal that the next symbol will be deflushed */
  int flag_flush=0;

public:


  ///////////////////////////////////////////////////////
  /* inizializza */
  Decoder(//! callback that will receive the decoded symbols
	  callback_t output_callback_ = NULL ,
	  //! callback for testing
	  callback_t bit_callback_    = NULL)
  {
    prefix="decoder";
    output_callback = output_callback_;
    bit_callback = bit_callback_;
    P("init"); PB("init");
    callback_data = this;
  }
private:
  ///////////////////////////////////////////////////////
  /* searches a S-interval containing the B-interval, if any
   * returns NO_SYMBOL if no symbol could be found
   */
  int search(//!  cumulative frequencies
	     I_t cum_freq[],
	     //! how many symbols
	     int max_symb )
  {
    I_t l,r; // left, right
    int s;
    PB("search");
    for(s=0; s<= (max_symb-1); s++) {
      r=interval_right(s , cum_freq);
      l=interval_left (s , cum_freq);
      assert(l <= r);
      if ( (Bhigh <= r ) &&   (Blow  >= l   ))      {
	PRINT("success symb %d S-interval  %s %s \n",s, string_binary(l).c_str(), string_binary(r).c_str() );
	return s + MIN_SYMBOL;
      }
      else {
	PRINT("fail symb %d S-interval  %s %s \n",s, string_binary(l).c_str(), string_binary(r).c_str() );
      }
    }
    return NO_SYMBOL;
  };

  ///////////////////////////////////////////////////////
  /* searches a S-interval containing the B-interval, if any ; using binary tree search ;
   * returns NO_SYMBOL  if no symbol could be found
   */
  int search_fast( //!  cumulative frequencies
		   I_t cum_freq[],
		   //! how many symbols
		   int max_symb )
  {
    PB("search fast");
    unsigned int  s, r, l;
    assert( cum_freq[max_symb] == 0);
    // find the lowest s such that   (Blow  >= l)
    // check that it true at the leftmost S-subinterval
    l = interval_left(max_symb-1, cum_freq);
    assert( l == Slow);
    if ( Blow < l ) { // if not, there is no way we can find the S-subinterval
      PRINT("failure early identyfing symb (leftmost S-interval  %s ... , Blow %s) \n", string_binary(l).c_str(), string_binary(Blow).c_str() );
      return NO_SYMBOL;
    }
    // check that what happens the rightmost S-subinterval
    l = interval_left(0, cum_freq);
    if ( Blow >= l ) {
      r=interval_right(0, cum_freq);
      if (Bhigh <= r ) {
	PRINT("success early identifying symb %d S-interval  %s %s \n",
	      MIN_SYMBOL, string_binary(l).c_str(), string_binary(r).c_str() );
	return MIN_SYMBOL;
      } else
	return NO_SYMBOL;
    }
    // OK we can binary search
    {int hi_s=max_symb-1, low_s = 0;
    while(1) {
      if ( hi_s <= (low_s+1) ) {
	s=hi_s; break;
      }
      unsigned int news = low_s + (hi_s - low_s) / 2;
      unsigned int newl = interval_left(news, cum_freq);
      if ( Blow >= newl)
	hi_s = news;
      else
	low_s = news;
    }}
    // check just in case
    l = interval_left(s, cum_freq);
    assert( Blow >= l);
    // check that this works
    r = interval_right(s, cum_freq);
    if ( Bhigh <= r )      {
      PRINT("success identifying symb %d S-interval  %s %s \n", s+MIN_SYMBOL, string_binary(l).c_str(), string_binary(r).c_str() );
      return s+MIN_SYMBOL;
    }
    else {
      PRINT("failure identyfing symb (guessed %d S-interval  %s %s) \n",s+MIN_SYMBOL, string_binary(l).c_str(), string_binary(r).c_str() );
    }
    return NO_SYMBOL;
  }

  /* returns a symbol (a number from MIN_SYMBOL up) if a symbol can be identified, returns NO_SYMBOL otherwise
   * this implements the standard method in arithmetic decoding
   */
  int output_symbol_standard(//! cumulative frequency table
			     I_t cp[],
			     //! number of symbols
			     I_t ms
			     )
  {
    if( ! (Slow <= Blow && Bhigh <= Shigh ) ) {
      printf("*** the B-interval is not contained in the S-interval! \n");
      print_state();
      abort();
    }

    int symb=search_fast(cp, ms);
    // we may check that they provide the same result
#ifdef AC_CHECK_FAST_SEARCH
    assert(  symb == search(cp, ms) );
#endif
    if( symb != NO_SYMBOL )  {
      /* we understood the correct symbol, now we mimick the encoder, so to arrive at the same S-interval */
      push_symbol(symb,cp);
      output_bits(bit_callback);
      return(symb);
    } else {
      PRINT(" undecidable\n");
      return( NO_SYMBOL );
    }
  };

public:
  ///////////////////////////////////////////////////////
  /*! returns a symbol (a number from MIN_SYMBOL up),
   * or NO_SYMBOL if no symbol could be identified;
   * or, if the decoder was deflushing, FLUSH_SYMBOL to signal
   *  that it deflushed succesfully
   */
  int output_symbol(//! cumulative frequency table; if NULL,
		    //! stored cumulative_frequencies will be used
		    I_t cum_freq[] = NULL,
		    //! number of symbols; if <0 , stored number will be used
		    I_t max_symb = -1
		    )
  {
    F_t *cp=    cum_freq; I_t ms=max_symb;
    if(cp == NULL || ms<0) {
      cp = cumulative_frequencies;
      ms = max_symbol;
    }

    if (flag_flush)
      { cp=cum_freq_flush; ms = n_symbols_flush;}

    int symb = output_symbol_standard(cp, ms);

    if (symb == NO_SYMBOL)
      return NO_SYMBOL;

    if (flag_flush) {
      // do not return this symbol, but rather  FLUSH_SYMBOL
      flag_flush=0;
      PRINT(" deflushed (via symbol %d)\n", symb);
      return( FLUSH_SYMBOL );
    }

    n_out_symbs++;
      return(symb);
  }

  ///////////////////////////////////////////////////////
  /* add bit to internal state ; call the callback output_callback() if symbols are decoded */
  void input_bit(int bit)
  {
    push_bit(bit);
    if(output_callback) {
      assert(cumulative_frequencies && max_symbol >= 1);
      int s;
      while ( NO_SYMBOL != (s = output_symbol( cumulative_frequencies, max_symbol) )) {
	output_callback(s, callback_data);
      }
    }
  };

  /*! if the decoder is not using callbacks, and if the encoder was
   *  flushed before the next symbols, then this should be called in
   *  the decoder to keep in sync ; bits from the encoder should be
   *  inserted in the decoder, and this should be called until it
   *  returns FLUSH_SYMBOL
   */
  int deflush()
  {
    return output_symbol(cum_freq_flush, n_symbols_flush );
  };

  /*! if instead the callback is used for the decoder output, then this should be called when it is known
   * that the encoder was flushed before the next symbol ; the callback will receive FLUSH_SYMBOL when
   * the deflushing is done
   */
  void prepare_for_deflush()
  {
    flag_flush=1;
    PRINT("start deflushing\n");
  };

};




/*! computes the cumulative cum_freq given the frequencies of symbols  */
void freq2cum_freq(F_t cum_freq[], F_t freq[], int max_symb, int assert_non_zero=1)
{
  int lp;
  F_t c=0;

  cum_freq[max_symb]=0;
  for(lp=max_symb-1; lp>=0; lp--)
    {
      if( assert_non_zero) assert(freq[lp] > 0);
      c += freq[lp];
      cum_freq[lp] = c;
    }

  if(cum_freq[0] >= MAX_FREQ)
    fprintf(stderr,"\
WARNING:\n\
 the sum of the frequencies is %ld\n\
 but the maximum safe limit is %ld !\n\
 You risk underflowing! You should use renormalize_frequencies()\n\
 or increase AC_representation_bitsize (currently %d)\n\n",
	    cum_freq[0], MAX_FREQ, AC_representation_bitsize);

}

/*! computes the cum_freq given the probabilities */
void prob2cum_freq(F_t cum_freq[], double prob[], int max_symb)
{
  int lp;
  F_t c=0,f=0;
  const double r = MAX_FREQ-max_symb;

  cum_freq[max_symb]=0;
  for(lp=max_symb-1; lp>=0; lp--)
    {
      assert(prob[lp] > 0);
      f = (F_t)ceil(prob[lp] * r );
      assert(f  > 0);
      c += f;
      cum_freq[lp]=f;
    }
}

  /*! compute the entropy given the frequencies */
//! reduces the frequencies if they are too high
//! returns 1 if the frequencies were renormalized
int  renormalize_frequencies(//! table of frequencies
			     AC::F_t * fr ,
			     //! number of symbols
			     int n_symbols,
			     // the sum of all frequencies, if 0 it will be recomputed
			     AC::F_t sum=0)
{
  if(sum==0) {
    for(int j=0 ; j<=   n_symbols-1  ; j++) {
      sum += fr[j];
    }}
  if( sum >= AC::MAX_FREQ) {
    for(int j=0 ; j<=   n_symbols-1  ; j++) {
      fr[j] = (fr[j]+1) / 2 ;
    }
    return 1;
  } else
    return 0;
}

double compute_entropy(F_t *freq, int MAX_SYMB)
{
  double e=0.0;
  F_t c=0;
  for(int j=0; j < MAX_SYMB; j++) {
    if( freq[j] <= 0 ) printf(" *** freq %d is %d ***\n", j, freq[j]);
    c+=freq[j];
  }
  for(int j=0; j < MAX_SYMB; j++) {
    double p= (double)freq[j] / (double)c;
    if( p>0)
      e += - p *     log2(p);
  }
  return e;
}


/////////////////

typedef  std::function<void(int,int,F_t,F_t,double)> printit_t;

void print_table_generic(//! prints the heading of a columns
			 std::function<void(int)>  head,
			 //! prints the content of columns
			 printit_t printit,
			 F_t *fr, F_t * cufr, int n_symbols, const int columns=6)
{
  // header, legenda
  for(int col=0 ; col < columns  ; col++) {
    head(col);
  }
  //table columns
  const int step = n_symbols / columns,
    end = step + ((step * columns < n_symbols)?1:0);
  for(int k=0 ; k<end  ; k+=1) {
    //table rows
    for(int col=0 ; col < columns  ; col++) {
      int j=k+col*end;
      AC::F_t f = fr[j];
	printit(col, j, f, cufr[j], f / (double)cufr[0]);
    }
  }
}


//! prints the table of frequencies
void print_table_freq(F_t *fr, F_t * cufr, int n_symbols, const int columns=6, FILE *out=stderr)
{

  std::function<void(int)>  head =
    [out,columns,n_symbols](int col)
    {
      fprintf(out, "sym frequ probab|");
      if(col == (columns-1)) fprintf(out,"\n");
    };

  printit_t    printit  =
    [out,columns,n_symbols](int col, int j, F_t f,F_t cf,double p)
    {
      if(j <= (n_symbols-1))
	fprintf(out,"%3d %5d %5.4f|",
		j + MIN_SYMBOL  , f , p);
      if(col == (columns-1)) fprintf(out,"\n");
    };

  print_table_generic(head, printit,
		      fr,  cufr,  n_symbols, columns);
  fprintf(out,"\n");
}

//! prints the table of frequencies, with ascii characters
void print_table_freq_ascii(F_t *fr, F_t * cufr, int n_symbols, const int columns=6, FILE *out=stderr)
{

  std::function<void(int)>  head =
    [out,columns,n_symbols](int col)
    {
      fprintf(out, "sym c frequ probab|");
      if(col == (columns-1)) fprintf(out,"\n");
    };

  printit_t    printit  =
    [out,columns,n_symbols](int col, int j, F_t f,F_t cf,double p)
    {
      if(j <= (n_symbols-1))
	fprintf(out,"%3d %c %5d %5.4f|",
		j + MIN_SYMBOL  , (j>=32 && j<126)? j:32,
		f , p);
      if(col == (columns-1)) fprintf(out,"\n");
    };

  print_table_generic(head, printit,
		      fr,  cufr,  n_symbols, columns);
  fprintf(out,"\n");
}


//! prints the table of frequencies and cumulative frequencies, with ascii symbols
void print_table_cum_freq_ascii(F_t *fr, F_t * cufr, int n_symbols, FILE *out=stdout, int columns=4)
{
  std::function<void(int)>  head =
    [out,columns,n_symbols](int col)
    {
      fprintf(out, "nnn c frequ proba cumfre|");
      if(col == (columns-1)) fprintf(out,"\n");
    };

  printit_t    printit  =
    [out,columns,n_symbols](int col, int j,F_t f,F_t cf,double p) {
    if(j <= (n_symbols-1))
      fprintf(out,"%3d %c %5d %4.3f %6d|",
	      j + MIN_SYMBOL  , (j>=32 && j<126)? j:32,
	      f , p, cf);
    if(col == (columns-1)) fprintf(out,"\n");
  };

  print_table_generic(head, printit,
		      fr,  cufr,  n_symbols, columns);
  fprintf(out,"\n");

}

//! prints the table of frequencies and cumulative frequencies
void print_table_cum_freq(F_t *fr, F_t * cufr, int n_symbols, FILE *out=stdout, const int columns=4)
{
  std::function<void(int)>  head =
    [out,columns,n_symbols](int col) {
    fprintf(out, "nnn frequ proba cumfre|");
    if(col == (columns-1)) fprintf(out,"\n");
  };

  printit_t    printit  =
    [out,columns,n_symbols](int col, int j,F_t f,F_t cf,double p) {
    if(j <= (n_symbols-1))
      fprintf(out,"%3d %5d %4.3f %6d|",
	      j + MIN_SYMBOL  , f , p, cf);
    if(col == (columns-1)) fprintf(out,"\n");
  };

 print_table_generic(head, printit,
		     fr,  cufr,  n_symbols, columns);
  fprintf(out,"\n");
}


} // namespace AC

