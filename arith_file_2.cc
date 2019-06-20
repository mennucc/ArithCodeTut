/*
 * @author Andrea C. G. Mennucci
 *
 * @file arith_file_2.cc
 *
 * @copyright (C) 2010-2019 Andrea C. G. Mennucci
 *
 * The new BSD License is applied to this software, see LICENSE.txt
 *
 * this simple programs compress or decompress a file
 * it keeps an updated table of symbol frequencies
 *
 * this version uses classes and callbacks
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


//////////////////////////////////////////////////////////////////
class FileBase {
public:
  char* filename;
  FILE *inp,  *out;

  const int  n_symbols=256;
  AC::F_t *cum_freq;
  AC::F_t *freq;

  FileBase(char* filename_, FILE *inp_, FILE *out_)  : filename(filename_) , inp(inp_), out(out_)
  {
    cum_freq = new AC::F_t[n_symbols+2];
    freq = new AC::F_t[n_symbols+1];
    for(int j=0;j<n_symbols;j++)
      freq[j]=1;
    AC::freq2cum_freq(cum_freq,freq,  n_symbols);
  }
};

class FileDecoder :  public FileBase {
public:

  size_t outsize = 0;

  AC::Decoder *D;

  void the_output_callback(int s, uint64_t symbcount)
  {
    assert( AC::NO_SYMBOL !=  s);
    s = s - AC::MIN_SYMBOL;
    if(outsize>0) {
      fputc(s,out);
      outsize --;
      if ((outsize & 0xfff) == 0) printf("left to decode %d          \r", outsize);
    }
    // update the frequency table for next upcoming symbol
    freq[s]++;
    AC::freq2cum_freq(cum_freq, freq,  n_symbols);
    // no need to deflush...
  };

  FileDecoder(char* filename_, FILE *inp_, FILE *out_) : FileBase( filename_, inp_, out_)
  {
    using namespace std::placeholders;
    D = new AC::Decoder( std::bind(& FileDecoder::the_output_callback, this, _1, _2) );
    // initialize the frequency tables, that are constant pointers (but the values will change when symbols are read)
    D->max_symbol = n_symbols;
    D->cumulative_frequencies = cum_freq;

  };

  void run()
  {
    // read header
    char h[10];
    size_t siz = fread(h,1,3,inp);
    h[4]=0;
    if(siz != 3 || 0 != strcmp("AC\n",h))
      { fprintf(stderr, "cannot read header, size %d, from input file %s \n", siz, filename); throw std::exception();}
    int j = fscanf(inp,"%x", &outsize);
    if ( j != 1)
      { fprintf(stderr, "cannot read size of output file from input file : %s\n", filename);  throw std::runtime_error("ciao %d");}
    else
      { printf("recovering file of size %d\n",outsize);  }
    {
      int b=fgetc(inp);
      assert( b == '\n');
    }
    while( outsize > 0 &&  !feof(inp) ) {
      int b;
      if ( !feof(inp) ) {
	b=fgetc(inp);
	if( b != EOF ) {
	  assert( b == '0' || b == '1');
	  D->input_bit(b-'0');
	}
      }
    }
    if(outsize == 0)
      printf("\n recovered\n");
    else
      printf("\n truncated?\n");
  }
};

/////////////////////////////////////////////////////////////////////////////



class FileEncoder :  public FileBase {
public:

  size_t insize;

  uint64_t output_bits=0;

  AC::Encoder *E=NULL;

  void the_output_callback(int b, uint64_t symbcount)
  {
    assert(b>=0);
    fputc(b+'0',out);output_bits++;
  };

  FileEncoder(char* filename_,  FILE *inp_, size_t insize_, FILE *out_) :
    FileBase( filename_, inp_, out_),     insize(insize_)
  {
    using namespace std::placeholders;
    E = new AC::Encoder( std::bind(& FileEncoder::the_output_callback, this, _1, _2) );
  };

  void run()
  {
    off_t  count = insize;
    // write header
    fprintf(out,"AC\n%0lx\n",insize);
    while( count > 0 ) {
      count--;
      int i= fgetc(inp);
      if ( i != EOF ) {
	E->input_symbol(i+AC::MIN_SYMBOL ,cum_freq);
	// update the frequency table before next symbol
	freq[i]++;
	AC::freq2cum_freq(cum_freq, freq,  n_symbols);
      }
      if ((count & 0xfff) == 0) printf("left to encode %d          \r", count);
    }
    E->flush();
    printf(" input %d symbols, output %d bit, ratio %2.1f %%\n", insize, output_bits, 100. * (double)output_bits/  (double)insize / 8. );
  }
};

/////////////////////////////////////////////////////////////////////////////

int print_help(char *cmd)
{
  printf("Usage: %s flag input output\n\
  -C encode\n\
  -D decode\n\
", cmd);
    return 0;
}

int
main(int argc, char * argv[])
{

  using namespace std::placeholders;


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


  if ( 0==strcmp(argv[1] , "-C") ) {
    /// ENCODER

    off_t insize =  statbuf.st_size;
    FileEncoder *    E = new FileEncoder(argv[2], inp, insize, out);
    E->run();

  } else   if ( 0==strcmp(argv[1] , "-D") ) {
    /// DECODER

    FileDecoder *    D = new FileDecoder(argv[2],inp,out);
    D->run();

  } else {
      fprintf(stderr,"Unrecognized option: %s\n\n",argv[1]);
      print_help(cmdname);
      return(-1);
    }

  fclose(inp);     fclose(out);
  return 0;
}
