/*
 * @author Andrea C. G. Mennucci
 *
 * @file arith_code.hh
 *
 * @copyright (C) 2010-2019 Andrea C. G. Mennucci
 *
 * The new BSD License is applied to this software, see LICENSE.txt
 *
 */



namespace AC {

#include "arith_code_config.hh"


/*********** general constants ****/

/*! types for variables defining intervals */
#if (AC_representation_bitsize <= 15)
typedef uint16_t    I_t;
typedef uint32_t  long_I_t;
#define AC_PRI_I_t  PRId16
#define AC_PRI_long_I_t  PRId32
#define AC_SIZE 16
#elif (AC_representation_bitsize <= 31)
typedef uint32_t    I_t;
typedef uint64_t  long_I_t;
//! macro for printf for I_t
#define AC_PRI_I_t  PRId32
//! macro for printf for long_I_t
#define AC_PRI_long_I_t  PRId64
#define AC_SIZE 32
#elif (AC_representation_bitsize <= 63)
typedef uint64_t        I_t;
typedef __int128  long_I_t;
#define AC_PRI_I_t  PRId64
#define AC_PRI_long_I_t  PRId128 //this does not exist...
#define AC_SIZE 64
#else
#error "ARITHMETIC CODEC says: too many bits"
#endif

/*! types for variables defining frequencies */
typedef  I_t F_t;
//! macro for printf for F_t
#define AC_PRI_F_t  AC_PRI_I_t
/*! the sum of all frequencies of symbols cannot exceed this value */
const  I_t  MAX_FREQ = ((I_t)1)      << (AC_representation_bitsize-2) ;

//! struct to represent an interval
typedef  std::pair<I_t, I_t>  interval_t;

//! type for callbacks
typedef std::function<void(int,void *)> output_callback_t;

//! type for bit reader call
typedef std::function<int(void *)> input_callback_t;


/**! base class ; should not be used; contains the logic
 * common to encoder and decoder
 */
class Base{

public:
  //! name of the class, for printing
  const char *prefix;

  //! stream where verbose output will be printed
  //! (verbose output may be enabled by defining the AC_verbose macro)
  FILE * verbose_stream = stdout;

  //! number of bits inserted in the state
  unsigned int number_input_bits();
  //! number of symbols inserted in the state
  unsigned int number_input_symbols();
  //! number of bits extracted from the state
  unsigned int number_output_bits();
  //! number of symbols extracted from the state
  unsigned int number_output_symbols();

  //! print the internal state
  void print_state();

  //! return the S_interval
  interval_t  S_interval();

  //! return the B_interval
  interval_t  B_interval();

  //! outputs one bit from the state, if available, else -1
  int output_bit();

  /*!  outputs multiple bits, returns them using a callback (if not null; else they are lost) */
  void output_bits(output_callback_t out);

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
  void frequencies2cumulative_frequencies();


  //! a pointer to data that the callbacks receive as second argument
  //! (it is initialized with a pointer to the class)
  void *callback_data = NULL;

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

  /*! cumulative tables of 8 equidistributed symbols */
  F_t const cum_freq_uniform8[9] =  { 8 , 7 , 6 , 5, 4 , 3 , 2 , 1 , 0 };

  /*! how many symbols in the  special cumulative table used for flushing */
  static const int n_symbols_flush = 8;

  /*! special cumulative table used for flushing */
  F_t const * cum_freq_flush = cum_freq_uniform8;

  // the old table was
  //F_t const cum_freq_flush[n_symbols_flush+1] =  { Qtr , QtrMinus , 1 , 0 };

  //! S-interval left extreme
  I_t  Slow;
  //! S-interval right extreme (included in the interval)
  I_t  Shigh;
  //! S-interval width
  long_I_t Srange;

  //! B-interval left extreme
  I_t  Blow;
  //! B-interval right extreme
  I_t Bhigh;

  // operations on intervals
  void  doubleit();
  void  doublehi();
  void  doublelow();
  void  doublecen();

  /*! divides Slow - Shigh in subintervals : returns the beginning of each interval;
    note that intervals are in reverse order wrt symbols, that is,
    symb=0 gives the rightmost subinterval,
    symb=max_symbols-1 is the leftmost */
  I_t separ_low_high(int symb,  const F_t  * cum_freq);
  
  /*! returns 0 , 1 or -1 if no bit can be pulled at this moment ; resize S-interval and B-interval accordingly */
  int  resize_pull_one_bit();
  
  /*! right extreme of a S-sub-interval ; note that symbols start from 0 here */
  I_t interval_right(int symb, const F_t * cum_freq);

  /*! left extreme of a S-sub-interval */
  I_t interval_left(int symb, const F_t * cum_freq);

  /*! put symbol in S-interval by splitting it and choosing a subinterval, proportional to the frequencies */
  void push_symbol(int symb, const F_t * cum_freq);

  /*! put bit in B-interval */
  void push_bit(int bit);

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

  Base();

};



/**************** ENCODER **********/
class Encoder : public Base { 

public:
  /*! callback when the encoder encodes a symbo */
  output_callback_t output_callback;

  /*! initialize, with a callback function that will output bits ;
      if  output_callback_ is NULL then the code should 'poll' the encoder by calling output_bit() */
  Encoder(//! callback that will receive the encoded bits
	  output_callback_t output_callback_ = NULL);

  /*! insert a symbol; if output_callback() was provided, send it all available bits */
  void input_symbol( //! symbol to add to the state!
		    int symb,
		    //! cumulative frequencies (if not provided, use the internally stored ones
		    const F_t * cum_freq = NULL);

  //! flush the encoder
  //! (if you are not using callbacks, be sure to pull all bits out of the encoder
  //! before and after flushing, or underflow may occour).
  //! The method for flushing is described in doc/on_deflushing.pdf 
  void flush();
};


/********************* DECODER **********/
class Decoder : public Base {

private:
  //
  int input_eof = 0;

  /* callback when the decoder decodes a symbol */
  output_callback_t output_callback;


  /* when this callback is called, the  S-interval in the decoder is the same as the
   * S-interval in the encoder (at the same bitcount)
   * This is used only for 
   */
  output_callback_t bit_callback;

  //! a callback to read bits
  input_callback_t read_bit_call;

  /* signal that the next symbol will be deflushed */
  int flag_flush=0;

  /*! searches a S-interval containing the B-interval, if any
   *! returns NO_SYMBOL if no symbol could be found
   */
  int search(//!  cumulative frequencies
	     const F_t * cum_freq,
	     //! how many symbols
	     int max_symb );

  /*! searches a S-interval containing the B-interval, if any ; using binary tree search ;
   *! returns NO_SYMBOL  if no symbol could be found
   */
  int search_fast(//!  cumulative frequencies
	     const F_t * cum_freq,
	     //! how many symbols
	     int max_symb );
  /*! returns a symbol (a number from MIN_SYMBOL up) if a symbol can be identified, returns NO_SYMBOL otherwise
   *! this implements the standard method in arithmetic decoding
   */
  int output_symbol_standard(//! cumulative frequency table
			     const F_t * cp,
			     //! number of symbols
			     I_t ms
			     );

public:

  /* inizializza */
  Decoder(//! callback that will receive the decoded symbols;
	  //! if it is NULL, then the output_symbol() may be used to get symbols
	  output_callback_t output_callback_ = NULL ,
	  //! call that the Decoder will call when it needs to read bits;
	  //! if it is NULL, then input_bit() may be used to insert bits
	  input_callback_t read_bit_call_ = NULL ,
	  //! callback for testing the code (mostly unneeded)
	  output_callback_t bit_callback_    = NULL
	  );

  /*! It returns a symbol (a number from MIN_SYMBOL up),
   *  or NO_SYMBOL if no symbol could be identified;
   *  or, if the decoder was deflushing, FLUSH_SYMBOL to signal
   *  that it deflushed succesfully.
   *  If the input_callback was provided, it will call it to get the needed bits.
   */
  int output_symbol(//! cumulative frequency table; if NULL,
		    //! stored cumulative_frequencies will be used
		    F_t const *  cum_freq = NULL,
		    //! number of symbols; if <0 , stored number will be used
		    I_t max_symb = -1
		    );

#if 0
  //! add a bit in the internal state
  void push_bit(int bit);
#endif

  ///////////////////////////////////////////////////////
  /*! Add bit to internal state ; the Decoder will call the callback output_callback() if symbols are decoded.
   *! Do not use this if the input_callback was provided to the Decoder.
   */
  void input_bit(int bit);

  /*! Run the decoder. Can only be used when both input and output callbacks are defined.
   *! Will run until the the input callback returns -1 .
   *! The output callback must take care of  cumulative_frequencies, max_symbol ;
   *! moreover the callback should call prepare_for_deflush() to deflush (if/when encoder flushed).
   */
  void run();

  /*! this must be called when it is known that the encoder was
   * flushed before the next symbol ; if the callback is used
   * for the decoder output, the callback will receive FLUSH_SYMBOL
   * when the deflushing is done; if instead the callback is not used,
   * then the program must call output_symbol() repeatedly until it
   * will receive FLUSH_SYMBOL . Calling this twice before FLUSH_SYMBOL
   * is received raises an error.
   */
  void prepare_for_deflush();

  //! tells if the decoder is deflushing
  int is_deflushing();
};



/***************** utilities ***********/
  
/*! computes the cumulative cum_freq given the frequencies of symbols  */
void freq2cum_freq(F_t cum_freq[], F_t freq[], int max_symb, int  assert_non_zero);

  
}
