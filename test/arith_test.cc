/*
 * @author Andrea C. G. Mennucci
 *
 * @file arith_test.cc
 *
 * @copyright (C) 2010-2019 Andrea C. G. Mennucci
 *
 * The new BSD License is applied to this software, see LICENSE.txt
 *
 *
 * This is a very complex code, that can be used to test most codepaths in the
 * arith_code.cc library.
 *
 */


#include "../arith_code.cc"

#include "math.h"
#include <string.h>

///////////////////////////////////////////////////////
///// tunables

// test decoder by using the callback
// if it is undefined , then the decoder is pulled for symbols
// #define DECODE_BY_CALLBACK

// test encoder by using the callback
// if it is undefined , then the encoder is pulled for bits
// #define ENCODE_BY_CALLBACK


// test the flushing calls;
// if defined, a flushing is performed every N symbols;
#define PERIODIC_FLUSHING 10001

// test the flushing calls;
// if defined as N, N flushings are performed at the end;
// flushing is particularly important in the --centest,
// without flushing no bit is ever emitted
#define END_FLUSHING 1


/********************************************************/

#ifdef VERBOSE
#define verboseprint(A,args...)  printf(A,##args);
#else
#define verboseprint(A...)
#endif



AC::Decoder * D=NULL;
AC::Encoder * E=NULL;


uint64_t alloc_for_n_bits=-1, alloc_for_n_symbs=-1;


int  *bits, *symbs;
uint64_t  symb_in_ptr=0, symb_out_ptr=0,
  bit_out_ptr=0, in_flushing=0, out_flushing=0;

AC::interval_t * S_intervals, * B_interval;

/* returns a random symbol uniform in N choices, starting  from 0 up to including (N-1)  */
int uniform_random(int N)
{
  return  (lrand48() % N);
}

///////////////

int is_time_to_flush(uint64_t s)
{
#ifdef PERIODIC_FLUSHING
  return ( s % PERIODIC_FLUSHING ) == 0 ;
#else
  return 0;
#endif
}
/* callback for decoder */
void decodeout(int dec, void *p)
{
  assert( p == D);
  unsigned int count = D->number_output_symbols();
#ifdef END_FLUSHING
  if(count >= alloc_for_n_symbs) {
    verboseprint(" deflushing after last symbol       \n");
    D->prepare_for_deflush();
  }
#endif

  if ( dec == AC::FLUSH_SYMBOL ) {
    out_flushing++;
    verboseprint(" received a successful flush\n");
    return ;
  }

  int e=0;
  symb_out_ptr++;

  verboseprint("decodeout : out symb[%d==%d]= %d \n", count,  symb_out_ptr ,dec);

  assert( count ==   symb_out_ptr);

#define U(A,B) if((A)!=(B))				\
    {printf("ERROR  %s= %ld instead of %s=%ld\n",\
	    __STRING(A),(long)(A),__STRING(B),(long)(B));e++;}


  /* controlla che il symb decodificato sia uguale a quello codificato*/
  if( symb_out_ptr <= alloc_for_n_symbs)
    U(dec, symbs[symb_out_ptr]);

  if(e)abort();


  if (
#ifdef PERIODIC_FLUSHING
        (( (1+symb_out_ptr) % PERIODIC_FLUSHING )  == 0 )
#else
      0
#endif
      ) {
    verboseprint(" prepare for deflushing before symbol %d        \n", 1+symb_out_ptr);
    D->prepare_for_deflush();
  }
}

/* callback for encoder */
void  encodeout(int b, void *p)
{
  assert( p == E);
  uint64_t count = E->number_output_bits();

  bit_out_ptr++;
  if(bit_out_ptr != count) {
    printf(" ********* DISALIGNED  %d %d *********** \n" , bit_out_ptr, count);
  }
  if( count <= alloc_for_n_bits) {
    bits[count]=b;
    S_intervals[count] = E-> S_interval();
  } else printf(" * receving more bits than we allocated for %d %d ********* \n", count, alloc_for_n_bits);

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
     while(AC::NO_SYMBOL != (s=D->output_symbol(D ->  cumulative_frequencies, D->max_symbol ))  ) {
       decodeout(s,  D);
     }
   }
#endif
}

uint64_t max_delay=0;

/* callback for decoder, when decoder is aligned with encoder */
void state_consistency_callback(int b, void *p)
{
  assert( p == D);
  uint64_t c = D->number_output_bits();

  uint64_t delay = bit_out_ptr - c;
  if(delay > max_delay) max_delay = delay ;
  verboseprint("decoder delay %d - %d = %d\n",   bit_out_ptr, c,  delay );

  if( c <= alloc_for_n_bits &&  b != bits[c]  ) {
    printf("******** ERROR ***\n");
    printf(" decoder thinks encoder output  bit %d as %d-th bit but it was %d \n", b, c, bits[c]);
    D->print_state();
    E->print_state();
    abort();
  }
  if(  c <= alloc_for_n_bits && (  S_intervals[c] != D-> S_interval() ) ) {
    printf("******** STATE NOT ALIGNED with encoder at bit %d ***\n" , c);
    D->print_state();
    printf("instead of\n");
    printf("       Slo %s Shi %s\n" ,
	   AC::string_binary(S_intervals[c].first).c_str(), AC::string_binary(S_intervals[c].second).c_str());

  }
};


#ifdef  ENCODE_BY_CALLBACK
void pull_encoder_repeatedly() {}
#else
void pull_encoder_repeatedly()
{
  // extract all bits
  int b;
  while(-1 != (b = E->output_bit() )) {
    encodeout(b, E);
  }
}
#endif





int print_help(char *cmd)
{
  printf("Usage: %s LEN flag n1 n2 n3 n4 ...\n\
   test arithmetic encoder with a stream of LEN i.i.d. symbols, generated as :\n \
 -U  N    uniformly distributed on N occurrences\n\
 -R  N    distributed according to a randomly chosen distribution\n\
 -S  n1 n2 n3 n4 n5 n6 ...\n\
     a stream of symbols distributed as 'n1 n2 n3 n4 n5 n6...' \n\
 --centest   tests with a sequence that forces maximum delay\n\
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
  int check_n_arguments =  argc >= 4  || ( argc == 3 && 0==strcmp(argv[2] , "--centest") );

  if ( ! check_n_arguments ) {
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
      freq[j]=uniform_random(1024)+1;
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
    } else  if  ( 0==strcmp(argv[1] , "--centest") ) {
      max_symb = 3;
      assert(max_symb>=1);
      // allocate
      freq     = new AC::F_t[max_symb];
      cum_freq = new AC::F_t[max_symb+1];
      // fill frequencies
      freq[0] = 1;       freq[1] = 2;       freq[2] =1;
    } else {
      fprintf(stderr,"Unrecognized option: %s\n\n",argv[1]);
      print_help(cmdname);
      return(-1);
    }

  printf("**  AC_representation_bitsize %d\n", AC_representation_bitsize);

  printf("** probability and entropy of given symbols \n");//////////////////////

  double entropy = AC::compute_entropy(freq,max_symb);
  printf(" entropy %g  \n", entropy );


  // compute cumulants
  AC::freq2cum_freq(cum_freq, freq, max_symb);
  assert( cum_freq [0] > 0 );

  if(max_symb < 72 ) {
    AC::print_table_cum_freq(freq , cum_freq,  max_symb, stdout);
  }
  // allocate
  alloc_for_n_symbs=LOOP;
  symbs = new int[alloc_for_n_symbs+1];
  alloc_for_n_bits = LOOP * ceil(entropy+1.) + AC_representation_bitsize * END_FLUSHING;
  bits =   new int[alloc_for_n_bits+1];
  S_intervals = new AC::interval_t[alloc_for_n_bits+1];

  printf("** prepare pseudo file \n");//////////////////////

  // fill pseudofile

  if  ( 0==strcmp(argv[1] , "--centest") ) {
    for(uint64_t k=1;k<=LOOP;k++) {
      symbs[k]=1+AC::MIN_SYMBOL;
    }
  } else
  if ( uniform_random_flag ) {
     for(uint64_t k=1;k<=LOOP;k++) {
       // randomly distributed from 1 to max_symb
       int r = uniform_random(max_symb) + AC::MIN_SYMBOL ;
       symbs[k]=r;
     }} else {
    for(uint64_t k=1;k<=LOOP;k++) {
      // randomly distributed from 1 to cum_freq[0]
      unsigned int r = uniform_random(cum_freq[0]);
      for(int j=   max_symb-1 ; j >= 0 ; j--) {
	if ( r < cum_freq[j] ) {
	  symbs[k]=j + AC::MIN_SYMBOL;
	  break;
      }
	if ( (k & 0xfffff) == 0)
	  printf("  (%2.1f %%) \r", 100. * k / (double) LOOP );
      }
    }
  }

  printf("** compute statistics \n"); //////////////////////

  AC::F_t * empirical_freq = new AC::F_t[max_symb],
    * empirical_cum_freq = new AC::F_t[max_symb] ;

  for(int j=   max_symb-1 ; j >= 0 ; j--)
    empirical_freq[j] = 0;

  for(symb_in_ptr=1;symb_in_ptr<=LOOP;symb_in_ptr++)
    empirical_freq[ symbs[symb_in_ptr] - AC::MIN_SYMBOL] ++ ;

  AC::freq2cum_freq(empirical_cum_freq, empirical_freq, max_symb, 0);

  if(max_symb < 72 ) {
    AC::print_table_cum_freq(empirical_freq , empirical_cum_freq,  max_symb);
  }

  double empirical_entropy = AC::compute_entropy(empirical_freq,max_symb);

  printf(" empirical entropy %g  \n", empirical_entropy );

  printf("** start encoding / decoding \n"); //////////////////////

#ifdef  DECODE_BY_CALLBACK
  D= new AC::Decoder(decodeout,state_consistency_callback);
#else
  D= new AC::Decoder(NULL,state_consistency_callback);
#endif

  // initialize the frequency tables, that are constant
  D->max_symbol = max_symb;
  D->cumulative_frequencies = cum_freq;

#ifdef  ENCODE_BY_CALLBACK
  E= new AC::Encoder(encodeout);
#else
  E= new AC::Encoder();
#endif

  // insert all symbols in the encoder
  for(symb_in_ptr=1;symb_in_ptr<=LOOP;symb_in_ptr++) {
    // maybe flush
#ifdef PERIODIC_FLUSHING
    if ( (symb_in_ptr % PERIODIC_FLUSHING )  == 0 ) {
      verboseprint("main : flushing \n", symb_in_ptr,symbs[symb_in_ptr]);
      E->flush();
      in_flushing++;
      pull_encoder_repeatedly();
    }
#endif
    // insert symbol in encoder
    {
      verboseprint("main : in symb[%d]= %d \n", symb_in_ptr,symbs[symb_in_ptr]);
      assert( AC::MIN_SYMBOL <= symbs[symb_in_ptr] && symbs[symb_in_ptr] <= (max_symb-1+AC::MIN_SYMBOL));
      E->input_symbol( symbs[symb_in_ptr] ,cum_freq);
      pull_encoder_repeatedly();
    }
  }// end of loop

  // correct for last value when exiting loop
  symb_in_ptr--;
  ///////////////////////////////// all the pseudo file was processed

#ifdef END_FLUSHING
  // flush encoder
  for(int j=0; j<   END_FLUSHING; j++) {
    verboseprint("main : endflushing %d\n",j);
    E->flush();
    in_flushing++;
    pull_encoder_repeatedly();
  }
#endif

#ifdef VERBOSE
  printf("\n*** internal state at end \n\n");
  E->print_state();
  D->print_state();
#endif

  printf("\n***\n");
  
#ifdef PERIODIC_FLUSHING
  printf(" entropy + overhead due to flushing %g  \n", entropy + log2(AC_representation_bitsize) / (double) PERIODIC_FLUSHING);
#endif

  printf(" symb_in_ptr %d symb_out_ptr %d bit_out_ptr %d \n" , symb_in_ptr,symb_out_ptr, bit_out_ptr);

  printf(" entropy %g ratio %g \n", entropy,  (double) bit_out_ptr / (double)(LOOP) );

  int64_t delay = (int64_t)bit_out_ptr -  (int64_t)D->number_output_bits();
  printf(" in_flushing %d  out_flushing %d delay at end %d max_delay %d\n",  in_flushing,  out_flushing, delay, max_delay);

  if ( symb_out_ptr < LOOP )
    printf("!!! COULD NOT DECODE  FINAL SYMBOL\n");

  return 0;
}
