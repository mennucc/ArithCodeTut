#include "arith_code.cc"
#include <stdio.h>
#include "common.cc"
int main()
{
  init();
  // decoder object
  AC::Decoder *D = new AC::Decoder() ;
  D->cumulative_frequencies = cum_freq;
  D->max_symbol = n_symbols;
  int s = AC::NO_SYMBOL ,  n=0;
  while( s != eof_symbol ) {
    if( ! feof(stdin) ) {
      //insert one bit
      int b = fgetc(stdin) - '0';
      D->input_bit(b);
    }
    // output all available symbols
    while( AC::NO_SYMBOL != (s = D->output_symbol() )) {
      if (s != eof_symbol) {
	int c = symbol_to_char(s);
	fprintf(stdout,"decoder received symbol[%d] = %d , i.e. character '%c' \n", n, s, c);
	n++;
      } else {
	fprintf(stdout,"decoder received EOF symbols\n");
	break;
      }
    }
  }
}
