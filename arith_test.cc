/*
 * author Andrea C. G. Mennucci
 *
 * Copyright (C) 2010-2019 Andrea C. G. Mennucci
 *
 * The new BSD License is applied to this software, see LICENSE.txt
 *
 */


#include "arith_code.cc"

#include "math.h"
#include <string.h>

///////////////////////////////////////////////////////
///// tunables

// test decoder by using the callback
// if it is undefined , then the decoder is pulled for symbols
#define DECODE_BY_CALLBACK

// test the flushing calls;
// if defined, a flushing is performed every N symbols;
#define PERIODIC_FLUSHING 10001

// test the flushing calls;
// if defined, a flushing is performed at the end;
// flushing is particularly important in the --centest,
// without flushing no bit is ever emitted
#define END_FLUSHING


/********************************************************/

AC::Decoder * D=NULL;
AC::Encoder * E=NULL;


uint64_t alloc_for_n_bits=-1, alloc_for_n_symbs=-1;


int  *bits, *symbs;
uint64_t  symb_in_ptr=0, symb_out_ptr=0,
  bit_out_ptr=0;

AC::I_t * Shighs, * Slows;

/* returns a random number between 1 and max */
int uniform_random(int max)
{
  return  (lrand48() % max) + 1;
}

/* callback for decoder */
void decodeout(int dec, uint64_t count)
{
  int e=0;
  symb_out_ptr++;
#ifdef VERBOSE
  printf("decodeout : out symb[%d==%d]= %d \n", count,  symb_out_ptr ,dec);
#endif

  assert( count ==   symb_out_ptr);

#define U(A,B) if((A)!=(B))				\
    {printf("ERROR  %s= %ld instead of %s=%ld\n",\
	    __STRING(A),(long)(A),__STRING(B),(long)(B));e++;}


  /* controlla che il symb decodificato sia uguale a quello codificato*/
  if( symb_out_ptr <= alloc_for_n_symbs)
    U(dec, symbs[symb_out_ptr]);

  if(e)abort();


  if (
#ifdef END_FLUSHING
      (symb_out_ptr >= alloc_for_n_symbs)
#else
      0
#endif
      ||
#ifdef PERIODIC_FLUSHING
        (( (1+symb_out_ptr) % PERIODIC_FLUSHING )  == 0 )
#else
      0
#endif
      ) {
#ifdef VERBOSE
    printf(" prepare for deflushing before symbol %d        \n", 1+symb_in_ptr);
#endif
    D->prepare_for_deflush();
  }
}

/* callback for encoder */
void  encodeout(int b, uint64_t count)
{
  bit_out_ptr++;
  if(bit_out_ptr != count) {
    printf(" ********* DISALIGNED  %d %d *********** \n" , bit_out_ptr, count);
  }
  if( count <= alloc_for_n_bits)
    bits[count]=b;
  //E->print_state();
  if( count <= alloc_for_n_bits ) {
    Shighs[count] = E-> Shigh;
    Slows[count]  = E-> Slow;
  } else printf(" ********** CAOS @(@*D!! %d %d ********* \n", count, alloc_for_n_bits);

#ifdef VERBOSE
  printf("encodeout : out bit[%d] = %d  feeding in decoder right now\n",  count, b);
#else
  if ( (count & 0xffff) == 0)
    printf(" bits[%6d]=%d  symbs[%6d]=%d ratio %g  (%2.1f %%) \r",
	   count, b,  symb_in_ptr, symbs[symb_in_ptr] ,
	   (double) bit_out_ptr / (double)(symb_in_ptr),
	   100. * symb_in_ptr/  (double)alloc_for_n_symbs);
#endif

  D->input_bit(b);

#ifndef  DECODE_BY_CALLBACK
   {
     int s;
     while(0 < (s=D->output_symbol(D ->  cumulative_frequencies, D->max_symbol ))  ) {
       decodeout(s,  symb_out_ptr+1);
     }
   }
#endif
}

/* callback for decoder, when decoder is aligned with encoder */
void state_consistency_callback(int b, uint64_t c)
{

  if( c <= alloc_for_n_bits &&  b != bits[c]  ) {
    printf("******** ERROR ***\n");
    printf(" decoder thinks encoder output  bit %d as %d-th bit but it was %d \n", b, c, bits[c]);
    D->print_state();
    E->print_state();
    abort();
  }
  if(  c <= alloc_for_n_bits && (  Shighs[c] != D-> Shigh  ||  Slows[c] != D-> Slow ) ) {
    printf("******** STATE NOT ALIGNED with encoder at bit %d ***\n" , c);
    D->print_state();
    printf("instead of\n");
    printf("       Slo %s Shi %s\n" ,
	   AC::string_binary(Slows[c]).c_str(), AC::string_binary(Shighs[c]).c_str());

  }
};





int print_help(char *cmd)
{
  printf("Usage: %s LEN flag n1 n2 n3 n4 ...\n\
   test arithmetic encoder with a stream of LEN i.i.d. symbols, generated as :\n \
 -U  N    uniformly distributed on N occurrences\n\
 -R  N    distributed according to a randomly chosen distribution\n\
 -S  n1 n2 n3 n4 n5 n6 ...\n\
     a stream of symbols distributed as 'n1 n2 n3 n4 n5 n6...' \n\
", cmd);
    return 0;
}

int
main(int argc, char * argv[])
{
  uint64_t LOOP;
  int max_symb;
  AC::F_t * freq;
  AC::F_t * cum_freq;


  char *cmdname = argv[0];

  if (argc <= 3) {
     print_help(cmdname);
     return 0;
   }

  LOOP=atof(argv[1]);
  assert(LOOP >= 2);

  argc--; argv++;

  srand48(1L);

  int uniform_random_flag = 0;

  ///////////////////////////////// read options, allocate memory
  if ( 0==strcmp(argv[1] , "-U") ) {
    uniform_random_flag = 1;
    max_symb=atof(argv[2]);
    // allocate
    freq     = new AC::F_t[max_symb];
    cum_freq = new AC::F_t[max_symb+1];
    // fill with uniform distribution
    for(int j=   max_symb-1 ; j >= 0 ; j--)
      freq[j]=1;
  } else  if ( 0==strcmp(argv[1] , "-R") ) {
    max_symb=atof(argv[2]);
    // allocate
    freq     = new AC::F_t[max_symb];
    cum_freq = new AC::F_t[max_symb+1];
    // fill with uniform distribution
    for(int j=   max_symb-1 ; j >= 0 ; j--)
      freq[j]=uniform_random(1024);
  } else
    if  ( 0==strcmp(argv[1] , "-S") ) {
      max_symb = argc -2;
      assert(max_symb>=1);
      // allocate
      freq     = new AC::F_t[max_symb];
      cum_freq = new AC::F_t[max_symb+1];
      // fill with given frequencies
      for(int i=0; i<max_symb; i++) {
	freq[i]=atof(argv[i+2]);
	assert( freq[i] > 0 );
      }
  } else {
      fprintf(stderr,"Unrecognized option: %s\n\n",argv[1]);
      print_help(cmdname);
      return(-1);
    }

  printf("** probability and entropy of given symbols \n");//////////////////////

  double entropy = AC::compute_entropy(freq,max_symb);
  printf(" entropy %g  \n", entropy );


  // compute cumulants
  AC::freq2cum_freq(cum_freq, freq, max_symb);
  assert( cum_freq [0] > 0 );

  if(max_symb < 10 ) {
    for(int j=   max_symb-1 ; j >= 0 ; j--)
      printf(" symb %d freq %d prob %g cumulant %d\n", j, freq[j] , freq[j]/(double)cum_freq[0], cum_freq[j]);
  }
  // allocate
  alloc_for_n_symbs=LOOP;
  symbs = new int[alloc_for_n_symbs+1];
  alloc_for_n_bits=LOOP * ceil(entropy+0.5);
  bits =   new int[alloc_for_n_bits+1];
  Shighs = new AC::I_t[alloc_for_n_bits+1];
  Slows  = new AC::I_t[alloc_for_n_bits+1];

  printf("** prepare pseudo file \n");//////////////////////

  // fill pseudofile

  if ( uniform_random_flag ) {
     for(uint64_t k=1;k<=LOOP;k++) {
       // randomly distributed from 1 to max_symb
       int r = uniform_random(max_symb);
       symbs[k]=r;
     }} else {
    for(uint64_t k=1;k<=LOOP;k++) {
      // randomly distributed from 1 to cum_freq[0]
      unsigned int r = uniform_random(cum_freq[0]);
      for(int j=   max_symb-1 ; j >= 0 ; j--) {
	if ( r <= cum_freq[j] ) {
	  symbs[k]=j+1;
	  break;
      }
	if ( (k & 0xfffff) == 0)
	  printf("  (%2.1f %%) \r", 100. * k / (double) LOOP );
      }
      assert( 1 <= symbs[k] && symbs[k] <= max_symb);
    }
  }

  printf("** compute statistics \n"); //////////////////////

  AC::F_t * empirical_freq = new AC::F_t[max_symb];

  for(int j=   max_symb-1 ; j >= 0 ; j--)
    empirical_freq[j] = 0;

  for(symb_in_ptr=1;symb_in_ptr<=LOOP;symb_in_ptr++)
    empirical_freq[ symbs[symb_in_ptr]-1 ] ++ ;

  if(max_symb < 10 ) {
    for(int j=   max_symb-1 ; j >= 0 ; j--)
      printf(" symb %d freq %d prob %g \n", j, empirical_freq[j] , empirical_freq[j]/(double)LOOP);
  }
  double empirical_entropy = AC::compute_entropy(empirical_freq,max_symb);

  printf(" empirical entropy %g  \n", empirical_entropy );

  printf("** start encoding / decoding \n"); //////////////////////

#ifdef  DECODE_BY_CALLBACK
  D= new AC::Decoder(decodeout,state_consistency_callback);
  // initialize the frequency tables, that are constant
#else
  D= new AC::Decoder(NULL,state_consistency_callback);
#endif

  D->max_symbol = max_symb;
  D->cumulative_frequencies = cum_freq;

  E= new AC::Encoder(encodeout);

  for(symb_in_ptr=1;symb_in_ptr<=LOOP;symb_in_ptr++) {
#ifdef PERIODIC_FLUSHING
    if ( (symb_in_ptr % PERIODIC_FLUSHING )  == 0 ) {
#ifdef VERBOSE
      printf("main : flushing \n", symb_in_ptr,symbs[symb_in_ptr]);
#endif
      E->flush();
      }
#endif
    {
#ifdef VERBOSE
      printf("main : in symb[%d]= %d \n", symb_in_ptr,symbs[symb_in_ptr]);
#endif
      E->input_symbol( symbs[symb_in_ptr] ,cum_freq);
    }
  }

  ///////////////////////////////// all the pseudo file was processed

#ifdef END_FLUSHING
  // flush encoder
  E->flush();
#endif

  printf("\n***\n symb_in_ptr %d symb_out_ptr %d bit_out_ptr %d \n" , symb_in_ptr,symb_out_ptr, bit_out_ptr);

#ifdef PERIODIC_FLUSHING
  printf(" entropy + overhead due to flushing %g  \n", entropy + log2(AC_representation_bitsize) / (double) PERIODIC_FLUSHING);
#endif

  printf(" entropy %g ratio %g \n", entropy,  (double) bit_out_ptr / (double)(LOOP) );

  return 0;
}
