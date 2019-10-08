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

/*! enable central zoom */
#define AC_QUARTER_ZOOM

