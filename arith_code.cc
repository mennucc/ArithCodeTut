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
#include <inttypes.h>

#include <functional>

#include "arith_code.hh"

namespace AC {

//////////////////////////////////////////////////////////////////
/// codes to output color

#if defined(AC_VERBOSE) && defined(AC_COLORED)
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#else
#define ANSI_COLOR_RED     ""
#define ANSI_COLOR_GREEN   ""
#define ANSI_COLOR_YELLOW  ""
#define ANSI_COLOR_BLUE    ""
#define ANSI_COLOR_MAGENTA ""
#define ANSI_COLOR_CYAN    ""
#define ANSI_COLOR_RESET   ""
#endif

//////////////////////////////////////////////////////////////////

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


#ifndef string_binary
#if AC_representation_bitsize <= 20
#define string_binary string_binary_comma
#else
#define string_binary string_hex
#endif
#endif

//! prints an integer in a string
std::string string_hex(I_t b)
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

//! prints a string in binary, as if it was a decimal number; used to
//! print the extreme of an interval
std::string string_binary_comma(I_t b, std::string sep="'")
{
  int s=AC_SIZE;
  std::string st;
  while(s>0) {
    st =  bit_rep[b & 0x0F] + st;
    b = b >> 4;
    s=s-4;
  }
  std::string so = "0,";
  for(int j=AC_SIZE - AC_representation_bitsize; j < AC_SIZE; j += 4) {
    so += st.substr(j,4) + sep;
  }
  return so;
}



/***********  stampa operazioni eseguite **************/

#ifdef AC_VERBOSE

#define AC_printINTERVAL(L,H)				\
  fprintf(verbose_stream, ANSI_COLOR_GREEN "%6s %s %6s %s "  ANSI_COLOR_RESET , \
	__STRING(L), string_binary(L).c_str(),			\
	__STRING(H), string_binary(H).c_str());

#define AC_printSHSL() if(verbose_stream) {AC_printINTERVAL(Slow,Shigh);}

#define AC_printBHBL() if(verbose_stream) {AC_printINTERVAL(Blow,Bhigh);}

#define AC_PRINT(A,args...) if(verbose_stream) {fprintf(verbose_stream, prefix); fprintf(verbose_stream," : "); fprintf(verbose_stream,A,##args);}

#define AC_PS(A) if(verbose_stream) {fprintf(verbose_stream,prefix); fprintf(verbose_stream," : ");AC_printSHSL();fprintf(verbose_stream," : " A "\n");}
#define AC_PB(A) if(verbose_stream) {fprintf(verbose_stream,prefix); fprintf(verbose_stream," : ");AC_printBHBL();\
    fprintf(verbose_stream," significant %d : " A "\n",significant_bits);}

#else
#define AC_PS(A)
#define AC_PB(A)
#define AC_PRINT(A...)
#endif



/* base class ; should not be used; contains the logic
 * common to encoder and decoder
 */
  // operations on intervals
  void  Base::doubleit()  { Slow = 2*Slow; Shigh = 2*Shigh+1; Srange=Shigh-Slow+1; assert(Srange>0); Blow=Blow*2; Bhigh = 2*Bhigh+1; }
  void  Base::doublehi()  { Slow -= Half; Shigh -= Half; Blow -= Half; Bhigh -= Half;  doubleit();  AC_PS("after doublehi"); }
  void  Base::doublelow() { doubleit(); AC_PS("after doublelow"); }
  void  Base::doublecen() { Slow -= Qtr; Shigh -= Qtr; Blow -= Qtr;Bhigh -= Qtr;  doubleit(); AC_PS("after doublecen"); }


  Base::Base() {
    n_in_bits=0; n_in_symbs=0; n_zooms=0; n_out_bits=0,  n_out_symbs=0;
    Blow = Slow = 0; Bhigh = Shigh=(I_t)(Top-1); Srange=Shigh-Slow+1;
    significant_bits=0;
#ifdef AC_QUARTER_ZOOM
    bitsToFollow=0;  virtual_bit=-1;
#endif
    prefix="base";
  };


  //! number of bits inserted in the state
  unsigned int Base::number_input_bits() { return n_in_bits ;};

  //! number of symbols inserted in the state
  unsigned int Base::number_input_symbols() { return n_in_symbs;};

  //! number of bits extracted from the state
  unsigned int Base::number_output_bits() { return n_out_bits;};

  //! number of symbols extracted from the state
  unsigned int Base::number_output_symbols() { return n_out_symbs;};

  //! print the internal state
  void Base::print_state()
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
  interval_t  Base::S_interval()
  {   interval_t i= {Slow, Shigh}; return i ; };

  //! return the B_interval
  interval_t  Base::B_interval()
  {   interval_t i= {Blow, Bhigh}; return i ; };

  /*! returns 0 , 1 or -1 if no bit can be pulled at this moment ; resize S-interval and B-interval accordingly */
  int  Base::resize_pull_one_bit()
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
	AC_PRINT("virtual bitsToFollow %d\n", bitsToFollow);
	significant_bits--;
	n_zooms++;
    }
#endif
    // at this point the width of the S-interval is more than 1/4
    // otherwise we would have returned a bit, or doublecen() many times
    assert(Srange > Qtr);
    return -1;
  };



  //! outputs one bit from the state, if available, else -1
  int Base::output_bit()
  {
#ifdef AC_QUARTER_ZOOM
    // are there virtual bits
    if(virtual_bit >= 0 &&  bitsToFollow > 0) {
      bitsToFollow--;
      n_out_bits++;
      AC_PRINT(" pull virtual bit %d, bits_to_follow %d\n", virtual_bit, bitsToFollow);
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
    AC_PRINT(" pull bit %d  (virtual bit  %d, bitsToFollow %d ) \n", b, virtual_bit, bitsToFollow);
    return b;
  }

  /*!  outputs multiple bits, returns them using a callback (if not null; else they are lost) */
  void Base::output_bits(output_callback_t out)
  {
    int b;
    while ( -1 != (b=resize_pull_one_bit()) ) {
      // is not  incremented in resize_pull_one_bit
      n_out_bits++;
      AC_PRINT(" output bit %d\n", b );
      if (out) out(b, callback_data);
#ifdef AC_QUARTER_ZOOM
      virtual_bit = 1 - b;
      while (0 < bitsToFollow) {
	n_out_bits++;
	bitsToFollow--;
	AC_PRINT(" output virtual bit %d, bits_to_follow %d\n", virtual_bit, bitsToFollow);
	if (out) out(virtual_bit, callback_data);
      }
      virtual_bit = -1;
#endif
    }
  };


  /*! divides Slow - Shigh in subintervals : returns the beginning of each interval;
     note that intervals are in reverse order wrt symbols, that is, 
     symb=0 gives the rightmost subinterval,
     symb=max_symbols-1 is the leftmost */
I_t Base::separ_low_high(int symb,  const F_t  * cum_freq)
  {
    assert( symb >= MIN_SYMBOL );
    //long_I_t Srange=(long_I_t)high-low+1;
    /* the following may OVERFLOW or UNDERFLOW */
    return  Slow + (I_t) ( ( Srange *  ((long_I_t)cum_freq[symb]) ) / ((long_I_t)cum_freq[0]) );
  }

/*! right extreme of a S-sub-interval ; note that symbols start from 0 here */
I_t Base::interval_right(int symb, const F_t * cum_freq) {
    return separ_low_high(symb,cum_freq)-1;
  };
  /*! left extreme of a S-sub-interval */
I_t Base::interval_left(int symb, const F_t * cum_freq) {
    return separ_low_high(symb+1,cum_freq);
  };

  /*! put symbol in S-interval by splitting it and choosing a subinterval, proportional to the frequencies */
void Base::push_symbol(int symb, const F_t * cum_freq)
  {
#ifdef AC_VERBOSE
    {
      F_t  f = cum_freq[symb] - cum_freq[symb+1], t = cum_freq[0];
      double p = (double) f / (double) t , e = - log2 (p) ;
      AC_PRINT(" put symb %d in S-interval, prob %f entropy %f  \n", symb, p ,  e );
    }
#endif
    I_t l,h;
    n_in_symbs++;
    h =  interval_right(symb,cum_freq);
    l =  interval_left (symb,cum_freq);
    Shigh = h;
    Slow = l;
    Srange = Shigh - Slow + 1;
    AC_PS("after push symbol");
    if(Slow>Shigh)
      {printf(" ************* S-interval underflow ***********\n");  print_state(); abort();}
  }

  /*! put bit in B-interval */
  void Base::push_bit(int bit)
  {
    AC_PRINT(" put bit %d in B-interval\n",bit);
    significant_bits++;
    n_in_bits++;
    if(bit==0)
      Bhigh=(Bhigh+1+Blow)/2-1;
    else
      Blow=(Bhigh+1+Blow)/2;
    AC_PB("after push bit");
    /* check for underflow */
    if(Blow>Bhigh)
      {printf(" ************* B-interval underflow ************\n");   print_state(); abort();}
  };

  void Base::frequencies2cumulative_frequencies()
  {
    assert(cumulative_frequencies &&  frequencies &&  max_symbol > 0);
    freq2cum_freq( cumulative_frequencies, frequencies,  max_symbol, 1);
  }




/**************** ENCODER **********/
Encoder::Encoder(//! callback that will receive the encoded bits
	  output_callback_t output_callback_ )
  { prefix=ANSI_COLOR_RED "encoder" ANSI_COLOR_RESET;
    output_callback = output_callback_;
    //this may go to the wrong stream
    //AC_PS("init"); AC_PB("init");
    callback_data = this;
  };

  /*! insert a symbol; if output_callback() was provided, send it all available bits */
void Encoder::input_symbol( //! symbol to add to the state!
		    int symb,
		    //! cumulative frequencies (if not provided, use the internally stored ones
		    const F_t * cum_freq )
  {
    if (cum_freq == NULL) {
      assert(cumulative_frequencies);
      cum_freq = cumulative_frequencies;
      assert(symb-MIN_SYMBOL < (int)max_symbol);
    }
    assert( symb >= MIN_SYMBOL );
    push_symbol(symb,cum_freq);
    if(output_callback) output_bits(output_callback);
  }

  //! flush the encoder
  //! (if you are not using callbacks, be sure to pull all bits out of the encoder
  //! before and after flushing, or underflow may occour).
  //! The method for flushing is described in doc/on_deflushing.pdf 
void Encoder::flush()
  {
    assert( Shigh >= Half && Half >= Slow );

    // flushing symbols are not counted
    n_in_symbs--;

    long_I_t a = Shigh;
    a = a + One;
    if ( Slow < Qtr ) {
      AC_PRINT(" start flushing, low\n");
      a = a - Qtr;
    } else {
      AC_PRINT(" start flushing, high\n");
      assert( Shigh >= (ThreeQtr-One) );
      a = a - Half;
    }
    a = a * 8;
    a = a / Srange;
    assert ( a >= 1 && a <= 8);
    int s = a - 1 ;
    input_symbol(s + MIN_SYMBOL, cum_freq_uniform8);
  };



/********************* DECODER **********/
  /* inizializza */
Decoder::Decoder(//! callback that will receive the decoded symbols
	  output_callback_t output_callback_  ,
	  //! call that the Decoder will call when it needs to read bits
	  input_callback_t read_bit_call_ ,
	  //! callback for testing
	  output_callback_t bit_callback_
	  )
  {
    prefix=ANSI_COLOR_BLUE "decoder" ANSI_COLOR_RESET;
    output_callback = output_callback_;
    bit_callback = bit_callback_;
    //this may go to the wrong stream
    //AC_PS("init"); AC_PB("init");
    callback_data = this;
    //
    read_bit_call =  read_bit_call_;
  };

///////////////////////////////////////////////////////
  /* searches a S-interval containing the B-interval, if any
   * returns NO_SYMBOL if no symbol could be found
   */
  int Decoder::search(//!  cumulative frequencies
	     const F_t *cum_freq,
	     //! how many symbols
	     int max_symb )
  {
    I_t l,r; // left, right
    int s;
    AC_PB("searching");
    for(s=0; s<= (max_symb-1); s++) {
      r=interval_right(s , cum_freq);
      l=interval_left (s , cum_freq);
      assert(l <= r);
      if ( (Bhigh <= r ) &&   (Blow  >= l   ))      {
	AC_PRINT("success symb %d S-interval  %s %s \n",s, string_binary(l).c_str(), string_binary(r).c_str() );
	return s + MIN_SYMBOL;
      }
      else {
	AC_PRINT("fail symb %d S-interval  %s %s \n",s, string_binary(l).c_str(), string_binary(r).c_str() );
      }
    }
    return NO_SYMBOL;
  };

  ///////////////////////////////////////////////////////
  /* searches a S-interval containing the B-interval, if any ; using binary tree search ;
   * returns NO_SYMBOL  if no symbol could be found
   */
int Decoder::search_fast( //!  cumulative frequencies
		   const F_t * cum_freq,
		   //! how many symbols
		   int max_symb )
  {
    AC_PB("fast searching");
    unsigned int  s, r, l;
    assert( cum_freq[max_symb] == 0);
    // find the lowest s such that   (Blow  >= l)
    // check that it true at the leftmost S-subinterval
    l = interval_left(max_symb-1, cum_freq);
    assert( l == Slow);
    if ( Blow < l ) { // if not, there is no way we can find the S-subinterval
      AC_PRINT("failure early identyfing symb (leftmost S-interval  %s ... , Blow %s) \n", string_binary(l).c_str(), string_binary(Blow).c_str() );
      return NO_SYMBOL;
    }
    // check that what happens the rightmost S-subinterval
    l = interval_left(0, cum_freq);
    if ( Blow >= l ) {
      r=interval_right(0, cum_freq);
      if (Bhigh <= r ) {
	AC_PRINT("success early identifying symb %d S-interval  %s %s \n",
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
      AC_PRINT("success identifying symb %d S-interval  %s %s \n", s+MIN_SYMBOL, string_binary(l).c_str(), string_binary(r).c_str() );
      return s+MIN_SYMBOL;
    }
    else {
      AC_PRINT("failure identyfing symb (guessed %d S-interval  %s %s) \n",s+MIN_SYMBOL, string_binary(l).c_str(), string_binary(r).c_str() );
    }
    return NO_SYMBOL;
  };

  /* returns a symbol (a number from MIN_SYMBOL up) if a symbol can be identified, returns NO_SYMBOL otherwise
   * this implements the standard method in arithmetic decoding
   */
int Decoder::output_symbol_standard(//! cumulative frequency table
			     const F_t * cp,
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
      AC_PRINT(" undecidable\n");
      return( NO_SYMBOL );
    }
  };


int Decoder::output_symbol(//! cumulative frequency table; if NULL,
		    //! stored cumulative_frequencies will be used
		    F_t const *  cum_freq,
		    //! number of symbols; if <0 , stored number will be used
		    I_t max_symb 
		    )
  {
    if(read_bit_call && input_eof == 0) {
      // it is more efficient to fill the state with bits before looking for symbols
      while(significant_bits <  AC_representation_bitsize) {
	int b = read_bit_call(callback_data);
	if( b == -1) {
	  input_eof = 1;
	  break;
	}
	Base::push_bit(b);
      }
    }
    F_t const * cp = cum_freq; I_t ms=max_symb;
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
      AC_PRINT(" deflushed (via symbol %d)\n", symb);
      return( FLUSH_SYMBOL );
    }

    n_out_symbs++;
      return(symb);
};


#if 0
void Decoder::push_bit(int bit)
  {
    assert( read_bit_call == NULL );
    Base::push_bit(bit);
  };
#endif

  ///////////////////////////////////////////////////////
  /* add bit to internal state ; call the callback output_callback() if symbols are decoded */
void Decoder::input_bit(int bit)
  {
    push_bit(bit);
    assert(NULL ==  read_bit_call);
    if(output_callback) {
      assert(cumulative_frequencies && max_symbol >= 1);
      int s;
      while ( NO_SYMBOL != (s = output_symbol( cumulative_frequencies, max_symbol) )) {
	output_callback(s, callback_data);
      }
    }
  };


void Decoder::run()
  {
    assert(output_callback && read_bit_call);
    assert(cumulative_frequencies && max_symbol >= 1);
    input_eof = 0;
    int s;
    while ( NO_SYMBOL != (s = output_symbol( cumulative_frequencies, max_symbol) )) {
      // this will call the input callback
      output_callback(s, callback_data);
    }
  };


  /*! this must be called when it is known that the encoder was
   * flushed before the next symbol ; if the callback is used
   * for the decoder output, the callback will receive FLUSH_SYMBOL
   * when the deflushing is done; if instead the callback is not used,
   * then the program must call output_symbol() repeatedly until it
   * will receive FLUSH_SYMBOL . Calling this twice before FLUSH_SYMBOL
   * is received raises an error.
   */
void Decoder:: prepare_for_deflush()
  {
    // if your program is working correctly, the flag should cleared
    // before this is called again
    assert(flag_flush==0);
    flag_flush=1;
    AC_PRINT("start deflushing\n");
  };

  //! tells if the decoder is deflushing
int Decoder::is_deflushing()
  {
    return flag_flush != 0 ;
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
 the sum of the frequencies is %" AC_PRI_F_t  " \n\
 but the maximum safe limit is %" AC_PRI_F_t  " !\n\
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


//! reduces the frequencies if they are too high
//! returns positive number if the frequencies were renormalized
int  renormalize_frequencies(//! table of frequencies
			     AC::F_t * fr ,
			     //! number of symbols
			     int n_symbols ,
			     //! maximum frequency
			     unsigned int max_freq = AC::MAX_FREQ
			     )
{
  int count=0;
  F_t sum=0;
  for(int j=0 ; j<=   n_symbols-1  ; j++) {
    sum += fr[j];
  }
  while( sum >= max_freq) {
    count++;
    for(int j=0 ; j<=   n_symbols-1  ; j++) {
      fr[j] = (fr[j]+1) / 2 ;
    }
    sum=0;
    for(int j=0 ; j<=   n_symbols-1  ; j++) {
      sum += fr[j];
    }
  }
  return count;
}

/*! compute the entropy given the frequencies */
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

