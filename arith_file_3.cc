/*
 * @author Andrea C. G. Mennucci
 *
 * @file arith_file_3.cc
 *
 * @copyright (C) 2010-2019 Andrea C. G. Mennucci
 *
 *
 *
 * this simple programs compress or decompress a file
 * it keeps an updated table of symbol frequencies
 *
 * this version is written in the style of C programs
 * and it uses a read callback for the Decoder
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


int print_help(char *cmd)
{
  printf("Usage: %s flag input output\n\
  -C encode\n\
  -D decode\n\
", cmd);
    return 0;
}


int read_a_bit(void *p)
{
  FILE * inp = (FILE *)p;
  int b;
  if ( !feof(inp) ) {
    b=fgetc(inp);
    if( b != EOF ) {
      assert( b == '0' || b == '1');
      return (b-'0');
    } else
      return -1;
  }
  return -1;
}


int
main(int argc, char * argv[])
{

  using namespace std::placeholders;

  AC::Decoder * D=NULL;
  AC::Encoder * E=NULL;

  char *cmdname = argv[0];

  int check_n_arguments =  argc == 4  ;

  if ( ! check_n_arguments ) {
     print_help(cmdname);
     return 0;
   }
  struct stat statbuf;


  { int j = stat(argv[3] , &statbuf );
    if ( j==0 && statbuf.st_size > 0 ) {
      fprintf(stderr,"Won't overwrite: %s\n\n",argv[3]);
      return -1;
  }}


  if ( 0 != stat(argv[2] , &statbuf )) {
      fprintf(stderr,"cannot stat : %s\n\n",argv[2]);
      perror(argv[2]);
      return -1;
  }

  if ( ! S_ISREG(statbuf.st_mode) ) {
      fprintf(stderr," is not a regular file: %s\n\n",argv[2]);
      return -1;
  }

  FILE  * inp = fopen(argv[2], "rb");
  if ( inp == NULL )  {
      fprintf(stderr,"Cannot open: %s\n\n",argv[2]);
      perror(argv[2]);
      return -1;
  }

  FILE  *out =  fopen(argv[3], "wb");
  if ( out == NULL )  {
      fprintf(stderr,"Cannot open: %s\n\n",argv[3]);
      perror(argv[3]);
      return -1;
  }

  const int  n_symbols=256;
  AC::F_t *cum_freq = new AC::F_t[n_symbols+2];
  AC::F_t *freq = new AC::F_t[n_symbols+1];
  for(int j=0;j<n_symbols;j++)
    freq[j]=1;
  AC::freq2cum_freq(cum_freq,freq,  n_symbols);

  if ( 0==strcmp(argv[1] , "-C") ) {
    //////////////////////////////////////////////////////////////////
    /// ENCODER
    uint64_t output_bits=0;
    off_t insize =  statbuf.st_size, count = insize;
    // write header
    fprintf(out,"AC\n%0lx\n",insize);
    E = new AC::Encoder() ;
    while( count > 0 ) {
      count--;
      int i= fgetc(inp);
      if ( i != EOF ) {
	E->input_symbol(i+AC::MIN_SYMBOL ,cum_freq);
	freq[i]++;
	AC::freq2cum_freq(cum_freq, freq,  n_symbols);
	int b;
	while(-1 != (b=E->output_bit())) {
	  fputc(b+'0',out); output_bits++;
	}
      }
      if ((count & 0xfff) == 0) printf("left to encode %d          \r", count);
    }
    E->flush();
    { int b;
      while(-1 != (b=E->output_bit())) {
	fputc(b+'0',out);output_bits++;
      }}
    printf(" input %d symbols, output %d bit, ratio %2.1f %%\n", insize, output_bits, 100. * (double)output_bits/  (double)insize / 8. );
  } else   if ( 0==strcmp(argv[1] , "-D") ) {
    //////////////////////////////////////////////////////////////////
    /// DECODER
    // read header
    off_t outsize=0;
    char h[10];
    size_t siz = fread(h,1,3,inp);
    h[4]=0;
    if(siz != 3 || 0 != strcmp("AC\n",h))
      { fprintf(stderr, "cannot read header, size %d, from input file %s \n", siz, argv[2]); return -1;}
    int j = fscanf(inp,"%x", &outsize);
    if ( j != 1)
      { fprintf(stderr, "cannot read size of output file from input file : %s\n", argv[2]); return -1;}
    else
      { printf("recovering file of size %d\n",outsize);  }
    {
      int b=fgetc(inp);
      assert( b == '\n');
    }
    //
    D= new AC::Decoder(NULL, NULL, read_a_bit);
    D->read_bit_call_data = inp;
    //
    while( outsize > 0 &&  !feof(inp) ) {
      int s;
      while( AC::NO_SYMBOL != (s = D->output_symbol(cum_freq, n_symbols) )) {
	s = s - AC::MIN_SYMBOL;
	if(outsize>0) {
	  fputc(s,out);
	  outsize --;
	  if ((outsize & 0xfff) == 0) printf("left to decode %d          \r", outsize);
	}
	freq[s]++;
	AC::freq2cum_freq(cum_freq, freq,  n_symbols);
	// no need to deflush...
      }
    }
    if(outsize == 0)
      printf("\n recovered\n");
    else
      printf("\n truncated?\n");
    /////////////////////////////////////////////////////////////////////////////
  } else {
      fprintf(stderr,"Unrecognized option: %s\n\n",argv[1]);
      print_help(cmdname);
      return(-1);
    }

  fclose(inp);     fclose(out);
  return 0;
}
