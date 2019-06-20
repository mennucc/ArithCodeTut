/*
 * author Andrea C. G. Mennucci
 *
 * Copyright (C) 2010-2019 Andrea C. G. Mennucci
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

// value that represents a non symbol
const int NO_SYMBOL = -1;
// minimum value for a symbol
// 0 is the common choice for computer programs
// 1 is the common choice for people
const int MIN_SYMBOL = 0;

// enable this to check that, in decoder, fast search and slow search always agree
// (the code will be much slower!)
// #define AC_CHECK_FAST_SEARCH

#ifndef  AC_representation_bitsize
// the number of bits to represent the intervals
#define AC_representation_bitsize 31
#endif

/* enable central zoom */
#define AC_QUARTER_ZOOM


/*********** general constants ****/

/* types for variables defining intervals */
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

/* types for variables defining frequencies */
typedef  I_t F_t;
/* the sum of all frequencies of symbols cannot exceed this value */
const  I_t  MAX_FREQ = ((I_t)1)      << (AC_representation_bitsize-2) ;



// type for callbacks
typedef std::function<void(int,uint64_t)> callback_t;


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
//#define ASFLOAT(H) ((double)((H))/(double)top)

//AC_print_binary

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




/* base class ; should not be used; contains the logic
 * common to encoder and decoder
 */
class Base{

public: //private:

#ifdef AC_QUARTER_ZOOM
  /* counts virtual bits, in case of centered zooms */
  unsigned int bitsToFollow;
  int virtual_bit;
#endif

  ////////// intervals manipulations
  const I_t  Top =  ((I_t)1   << AC_representation_bitsize);
  const I_t  Qtr =  ((I_t)1   << (AC_representation_bitsize-2));
  const I_t  Half = (Qtr*2);
  const I_t  ThreeQtr = (Qtr*3);

  // S-interval
  I_t  Slow,Shigh, Srange;
  // B-interval
  I_t  Blow,Bhigh;

  // operations on intervals
  void  doubleit()  { Slow = 2*Slow; Shigh = 2*Shigh+1; Srange=Shigh-Slow+1; assert(Srange>0); Blow=Blow*2; Bhigh = 2*Bhigh+1; }
  void  doublehi()  { Slow -= Half; Shigh -= Half; Blow -= Half; Bhigh -= Half;  doubleit();  P("doublehi"); }
  void  doublelow() { doubleit(); P("doublelow"); }
  void  doublecen() { Slow -= Qtr; Shigh -= Qtr; Blow -= Qtr;Bhigh -= Qtr;  doubleit(); P("doublecen"); }

public:
  // name of the class, for printing
  const char *prefix;

  // significant bits (used in the decoder)
  unsigned int significant_bits;

  // counters for number of operations
  unsigned int n_in_bits, n_in_symbs, n_zooms, n_out_bits, n_out_symbs;

  Base() {
    n_in_bits=0; n_in_symbs=0; n_zooms=0; n_out_bits=0,  n_out_symbs=0;
    Blow = Slow = 0; Bhigh = Shigh=(I_t)(Top-1); Srange=Shigh-Slow+1;
    significant_bits=0;
#ifdef AC_QUARTER_ZOOM
    bitsToFollow=0;  virtual_bit=-1;
#endif
    prefix="base";
  };

  // print the internal state
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
  //private:

private:
  /* returns 0 , 1 or -1 if no bit can be pulled at this moment ; resize S-interval and B-interval accordingly */
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
      	PRINT("virtual bitsToFollow %d\n", bitsToFollow);
      	doublecen();
	significant_bits--;
	n_zooms++;
    }
#endif
    return -1;
  };

public:

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
    PRINT(" pull bit %d\n",b);
    return b;
  }

  /* outputs multiple bits, returns them using a callback (if not null; else they are lost) */
  void output_bits(callback_t out)
  {
    int b;
    while ( -1 != (b=resize_pull_one_bit()) ) {
      // is not  incremented in resize_pull_one_bit
      n_out_bits++;
      if (out) out(b, n_out_bits);
#ifdef AC_QUARTER_ZOOM
      virtual_bit = 1 - b;
      while (0 < bitsToFollow) {
	n_out_bits++;
	if (out) out(virtual_bit, n_out_bits);
	bitsToFollow--;
	PRINT(" output virtual bit %d, bits_to_follow %d\n", virtual_bit, bitsToFollow);
      }
      virtual_bit = -1;
#endif
    }
  };

private:
  /* divides Slow - Shigh in subintervals : returns the beginning of each interval;
     note that intervals are in reverse order wrt symbols, that is, 
     symb=1 gives the rightmost subinterval,
     symb=max_symbols is the leftmost */
  I_t separ_low_high(int symb, I_t cum_freq[])
  {
    assert( symb >= MIN_SYMBOL );
    //long_I_t Srange=(long_I_t)high-low+1;
    /* the following may OVERFLOW or UNDERFLOW */
    return  Slow + (I_t) ( ( ((long_I_t)Srange) *  ((long_I_t)cum_freq[symb-MIN_SYMBOL]) ) / ((long_I_t)cum_freq[0]) );
  }
public:
  /* right extreme of a S-sub-interval */
  I_t interval_right(int symb, I_t cum_freq[]) {
    return separ_low_high(symb,cum_freq)-1;
  };
  /* left extreme of a S-sub-interval */
  I_t interval_left(int symb, I_t cum_freq[]) {
    return separ_low_high(symb+1,cum_freq);
  };


  /* put symbol in Slow - Shigh by splitting it and choosing a subpiece*/
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

  /* put bit in Blow - Bhigh*/
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
};



/**************** ENCODER **********/
class Encoder : public Base { 

public:

  /* callback when the encoder encodes a symbo */
  callback_t output_callback;

  /* callback to see if the decoder and encoder are in the same state when the decoder deduces that the encoder
     would have encoded the n-th bit */
  //void (*bit_callback)(int bit, int bitcount);
  ///void (*symbol_callback_)(int bit, int bitcount) = NULL)

  Encoder(callback_t output_callback_ = NULL)
  /* initialize, with a callback function that will output bits */
  { prefix="encoder";
    output_callback = output_callback_;
    P("init"); PB("init"); }

  /* insert a symbol; if output_callback() was provided, send it all available bits */
  void input_symbol(int symb, I_t cum_freq[])
  {
    assert( symb >= MIN_SYMBOL );
    push_symbol(symb,cum_freq);
    if(output_callback) output_bits(output_callback);
  }

  void flush()
  {
    F_t cum_freq_flush[3] = { Qtr, 1 , 0};
    input_symbol(1+MIN_SYMBOL, cum_freq_flush);
  }
};


/********************* DECODER **********/
class Decoder : public Base {



  /* callback when the decoder decodes a symbol */
  callback_t output_callback;


  /* special cumulative table used for flushing */
  F_t cum_freq_flush[3] = { Qtr, 1 , 0};
  /* signal that the next symbol will be deflushed */
  int flag_flush=0;

  /* when this callback is called, the  S-interval in the decoder is the same as the
   * S-interval in the encoder (at the same bitcount)
   * This is used only for 
   */
  callback_t bit_callback;

public:

  /* if the output callback is used, then these must be updated after
     each symbol is decoded, with the correct frequencies
     for the next symbol  */
  F_t * cumulative_frequencies  = NULL;
  I_t max_symbol = -1;

  ///////////////////////////////////////////////////////
  /* inizializza */
  Decoder(callback_t output_callback_ = NULL ,
	  callback_t bit_callback_    = NULL)
  {
    prefix="decoder";
    output_callback = output_callback_;
    bit_callback = bit_callback_;
    P("init"); PB("init");
  }
private:
  ///////////////////////////////////////////////////////
  /* searches a S-interval containing the B-interval, if any
   * returns NO_SYMBOL if no symbol could be found
   */
  int search( I_t cum_freq[], int max_symb)
  {
    I_t l,r; // left, right
    int s;
    PB("search");
    for(s=MIN_SYMBOL; s<= (max_symb-1+MIN_SYMBOL); s++) {
      r=interval_right(s , cum_freq);
      l=interval_left (s , cum_freq);
      assert(l <= r);
      if ( (Bhigh <= r ) &&   (Blow  >= l   ))      {
	PRINT("success symb %d S-interval  %s %s \n",s, string_binary(l).c_str(), string_binary(r).c_str() );
	return s;
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
  int search_fast( I_t cum_freq[], int max_symb)
  {
    PB("search fast");
    unsigned int  s, r, l;
    // find the lowest s such that   (Blow  >= l)
    // check that it true at the leftmost S-subinterval
    l = interval_left(max_symb-1+MIN_SYMBOL, cum_freq);
    if ( Blow < l ) { // if not, there is no way we can find the S-subinterval
      PRINT("failure early identyfing symb (leftmost S-interval  %s ... , Blow %s) \n", string_binary(l).c_str(), string_binary(Blow).c_str() );
      return NO_SYMBOL;
    }
    // check that what happens the rightmost S-subinterval
    l = interval_left(MIN_SYMBOL, cum_freq);
    if ( Blow >= l ) {
      r=interval_right(MIN_SYMBOL, cum_freq);
      if (Bhigh <= r ) {
	PRINT("success early identifying symb %d S-interval  %s %s \n",
	      MIN_SYMBOL, string_binary(l).c_str(), string_binary(r).c_str() );
	return MIN_SYMBOL;
      } else
	return NO_SYMBOL;
    }
    // OK we can binary search
    {int hi_s=max_symb-1+MIN_SYMBOL, low_s = MIN_SYMBOL;
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
      PRINT("success identifying symb %d S-interval  %s %s \n",s, string_binary(l).c_str(), string_binary(r).c_str() );
      return s;
    }
    else {
      PRINT("failure identyfing symb (guessed %d S-interval  %s %s) \n",s, string_binary(l).c_str(), string_binary(r).c_str() );
    }
    return NO_SYMBOL;
  }
public:
  ///////////////////////////////////////////////////////
  /* returns a symbol (a number from 1 to max_symb) if a symbol can be identified, returns zero otherwise */
  int output_symbol(I_t cum_freq[], I_t max_symb)
  {
    F_t *cp=    cum_freq; I_t ms=max_symb;
    // override, we are duflushing
    if (flag_flush)
      { cp=cum_freq_flush; ms=2;}

    int symb=search_fast(cp, ms);
    // we may check that they provide the same result
#ifdef AC_CHECK_FAST_SEARCH
    assert(  symb == search(cp, ms) );
#endif
    if( symb != NO_SYMBOL )  {
      /* we understood the correct symbol, now we mimick the encoder, so to arrive at the same S-interval */
      push_symbol(symb,cp);
      output_bits(bit_callback);
      if (flag_flush) {
	assert(symb == 1+MIN_SYMBOL);
	// do not return this symbol
	flag_flush=0;
	PRINT(" deflushed\n");
	return( NO_SYMBOL );
      } else {
	n_out_symbs++;
	return(symb);
      }
    } else {
      PRINT(" undecidable\n");
      return( NO_SYMBOL );
    }
  };

  ///////////////////////////////////////////////////////
  /* add bit to internal state ; call the callback output_callback() if symbols are decoded */
  void input_bit(int bit)
  {
    push_bit(bit);
    if(output_callback) {
      assert(cumulative_frequencies && max_symbol >= 1);
      int s;
      while ( NO_SYMBOL != (s = output_symbol( cumulative_frequencies, max_symbol) )) {
	output_callback(s, n_out_symbs);
      }
    }
  };

  /* if the encoder was flushed, then this should be called in the decoder to keep in sync ;
   * bits from the encoder should be inserted in the decoder, and this should be called  until it returns 1+AC::MIN_SYMBOL
   */
  int deflush()
  {
    return output_symbol(cum_freq_flush, 1+AC::MIN_SYMBOL );
  };

  /* if instead the callback is used for the decoder output, then this should be called when it is known
   * that the encoder was flushed before the next symbol */
  void prepare_for_deflush()
  {
    flag_flush=1;
  };

};




/* computes the cumulative cum_freq given the frequencies of symbols  */

void freq2cum_freq(F_t cum_freq[], F_t freq[], int max_symb)
{
  int lp;
  F_t c=0;

  cum_freq[max_symb]=0;
  for(lp=max_symb-1; lp>=0; lp--)
    {
      assert(freq[lp] > 0);
      c += freq[lp];
      cum_freq[lp] = c;
    }

  if(cum_freq[0] >= MAX_FREQ)
    printf("\
PROBLEMA: il codificatore aritmetico non ha precisione sufficiente\n\
 per gestire questa tabella di frequenze, cum %ld max %ld !\n\
 RISCHIO DI UNDERFLOW\n\n\n", cum_freq[0], MAX_FREQ);

}

/* computes the cum_freq given the probabilities */
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

}
