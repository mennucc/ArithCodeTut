/*
 * author Andrea C. G. Mennucci
 *
 * Copyright (C) 2010-2019 Andrea C. G. Mennucci
 *
 * The new BSD License is applied to this software, see LICENSE.txt
 *
 * This tests the code "in reverse"
 * It uses the decoder to "decode" a random string of bits;
 * the decoder outputs a string of symbols distributed as requested;
 * the encoder then reencodes them to recover the original string of bits.
 */


#include "arith_code.cc"

#include "math.h"
#include <string.h>

/********************************************************/

AC::Decoder * D=NULL;
AC::Encoder * E=NULL;

#define MAX_SYMB 5

#define LOOP 10000000

AC::F_t freq[MAX_SYMB]={2,2,80,2,1},
  cum_freq[MAX_SYMB+1];

int  bits[LOOP+1], bit_in_ptr=1, bit_out_ptr=bit_in_ptr;

AC::interval_t B_intervals[LOOP+1];

void  decodeout(int s, void *p)
{
  assert( p == D);
#ifdef  VERBOSE
  uint64_t count = D->number_output_symbols();
  printf("decodeout : out symb[%d] = %d \n",count, s);
#endif
  E->input_symbol(s, cum_freq);
}

void encodeout(int dec, void *p)
{
  assert( p == E);
  int e=0;
#ifdef  VERBOSE
  uint64_t count = E->number_output_bits();
  printf("encodeout : out bit[%d==%d]= %d \n", bit_out_ptr, count, dec);
#endif

#define U(A,B) if((A)!=(B)) \
    {printf("ERRORE  %s= %ld invece di %s=%ld\n",\
             __STRING(A),(long)(A),__STRING(B),(long)(B));e++;}

  /* controlla che il bit decodificato sia uguale a quello codificato*/
  U(dec, bits[bit_out_ptr]);

  /* queste uguaglianze non sono soddisfatte... perché ? */
  //U(highs[bit_out_ptr] , E->Bhigh);
  //U(lows[bit_out_ptr] , E->Blow);

  if(e)abort();

  bit_out_ptr++; 
}


int
main(int argc, char * argv[])
{
  int a;
  if (argc!=2) {
    printf("Usage: %s arg\n\
 where arg is\n\
 -1  test decoder with a random string of bits \n\
  n  test decoder with a string of repetitions of bit n\n",argv[0]);
    return 0;
  }

  a=(int)(atof(argv[1]));

  if( a< -1 || a > 1)  { printf("arg out of range\n"); return 0;}  

  AC::freq2cum_freq(cum_freq, freq, MAX_SYMB);

  D= new AC::Decoder(decodeout);
  E= new AC::Encoder(encodeout);

  D->max_symbol = MAX_SYMB;
  D->cumulative_frequencies = cum_freq;


  srand48(1L);

  for(;bit_in_ptr<=LOOP;bit_in_ptr++) {
    if (a == -1)
      bits[bit_in_ptr]=lrand48() % 2;
    else
      bits[bit_in_ptr]=a;

#ifndef VERBOSE
    if ( (bit_in_ptr & 0xfff ) == 1 )
      printf(" ( %2.1f%% )  \r", 100.0 * bit_in_ptr / (double)LOOP);
#else
    printf("main : in bit[%d]= %d \n", bit_in_ptr,bits[bit_in_ptr]);
#endif

    D->input_bit( bits[bit_in_ptr] );
    /* save state */
    B_intervals[bit_in_ptr]=D->B_interval() ;
  }
  printf("success                     \n");
  return 0;
}
