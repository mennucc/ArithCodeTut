/*
 * @author Andrea C. G. Mennucci
 *
 * @file arith_simple_markov.cc
 *
 * @copyright (C) 2019 Andrea C. G. Mennucci
 *
 * The new BSD License is applied to this software, see LICENSE.txt
 *
 * The  simple program  compresses or decompresses standard input to standard output;
 * it uses a simple Markov model for probabilities;
 * it keeps multiple tables of symbol frequencies;
 * it uses the previous symbol as a predictor for the next symbol;
 * it updates the tables when it encounters a symbol.
 *
 * Note that the encoded file is actually a text file, where bits are written
 * as characters '0' and '1'
 */


#include "arith_code.cc"

#include <functional>

#include <stdio.h>
#include "math.h"
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


// initialize the frequency tables supposing that the input is text
#define  TEXTLIKE_PROBABILITIES


//////////////////////////////////////////////////////////////////

const int  n_symbols=257, my_eof=256, freq_boost = 32;

AC::F_t **cum_freq ;
AC::F_t **freq_ ;

void markov_update(//! previous symbol
		   int p,
		   //! current symbol
		   int s)
{
  // boost frequency of seen symbols
  freq_[p][s] += freq_boost;
  // reduce the frequencies if too high for the encoder precision
  AC::renormalize_frequencies(freq_[p], n_symbols);
  // update the cumulative_frequencies from the frequencies
  AC::freq2cum_freq(cum_freq[p], freq_[p], n_symbols);
}


int flag_eof=0;

int prev_symbol=' ';


void decoder_callback(int s, void *p)
{
  AC::Decoder * D =  (AC::Decoder *)p;
  assert( AC::NO_SYMBOL !=  s);
  s = s - AC::MIN_SYMBOL;
  if(flag_eof) {
    ; // may receive a FLUSH_SYMBOL here
  } else if( s != my_eof ) {
    // good symbol
    fputc(s, stdout);
    // update frequency tables
    markov_update(prev_symbol, s);
    // set table for next symbol, using current symbol as predictor
    D->cumulative_frequencies = cum_freq[s];
    // shift one
    prev_symbol = s;

  } else {
    // we reached end of file
    flag_eof = 1;
    D->prepare_for_deflush();
  }
}

/////////////////////////////////////////////////////////////////////////////

void encoder_callback(	int b, void *p)
{
  fputc(b+'0',stdout);
}


/////////////////////////////////////////////////////////////////////////////

int print_help(char *cmd)
{
  printf("Usage: %s flag \n\
\n\
The  simple program  compresses or decompresses standard input to standard output\n\
Note that the encoded output is actually text, where bits are written\n\
as characters '0' and '1'.\n\
\n\
Flags:\n\
  -C encode\n\
  -D decode\n\
", cmd);
    return 0;
}

#include "frequencies.cc"

#ifdef TEXTLIKE_PROBABILITIES
#define initialize_frequencies initialize_frequencies_dickens
#else
#define initialize_frequencies initialize_frequencies_nonnull
#endif


int
main(int argc, char * argv[])
{
  char *cmdname = argv[0];

  int check_n_arguments =  argc == 2  ;

  if ( ! check_n_arguments ) {
     print_help(cmdname);
     return 0;
   }

  // initialize frequency tables
  cum_freq = new AC::F_t* [n_symbols];
  freq_    = new AC::F_t* [n_symbols];
  for(int j=0;j < n_symbols;j++) {
    cum_freq[j] = new AC::F_t[n_symbols];
    freq_[j]    = new AC::F_t[n_symbols];
    initialize_frequencies(freq_[j]);
    AC::freq2cum_freq(cum_freq[j], freq_[j],  n_symbols);
  }


  if ( 0==strcmp(argv[1] , "-C") ) {
    //////////////// ENCODER
    AC::Encoder *E = new AC::Encoder(encoder_callback) ;
    E -> verbose_stream = stderr;
    int s;
    while(EOF !=  ( s = fgetc(stdin) )) {
      // input symbol in encoder
      assert( prev_symbol !=  EOF);
      //printf("[%d]" , prev_symbol);
      E->input_symbol(s+AC::MIN_SYMBOL, cum_freq[prev_symbol]);
      // update frequency tables
      markov_update(prev_symbol, s);
      // shift one
      prev_symbol = s;
    }
    // send eof
    E->input_symbol(my_eof + AC::MIN_SYMBOL, cum_freq[prev_symbol]);
    E->flush();
    printf("\n");

    {
      uint64_t ns = E->number_input_symbols(), nb = E->number_output_bits();
      fprintf(stderr," input symbols %d (including eof, excluding flushing); output bits %d,\n ratio %f bits per symbol\n",
	      ns,nb,(double)nb/(double)ns);
    }
    /////////////////// end of encoder code
  }

  else    if ( 0==strcmp(argv[1] , "-D") )

    {
    //////////////////// DECODER
    AC::Decoder * D= new AC::Decoder(decoder_callback);
    D -> verbose_stream = stderr;
    // we must store frequencies in decoder, since the decoder
    // must know them in advance for each symbol that it is trying to decode
    D->cumulative_frequencies = cum_freq[prev_symbol];
    D->max_symbol = n_symbols;
    int b;
    while(EOF !=  ( b = fgetc(stdin) ) ) {
      if( b == '0' || b == '1')
	D->input_bit(b-'0');
      else if (b>=32 && b <= 126)
	fprintf(stderr, "input '%c' ignored\n", b);
    }
    if(!flag_eof)
      fprintf(stderr,"\n*** internal error, did not receive EOF, \n");
    ////////////////
  }

  else {
    fprintf(stderr,"Unrecognized option: %s\n\n",argv[1]);
    print_help(cmdname);
    return(-1);
  }

#ifdef PRINT_TABLE
  fprintf(stderr,"*******\nfinal table\n");
  AC::print_table_cum_freq_ascii(freq, cum_freq, n_symbols, stderr);
#endif

  return 0;
}
