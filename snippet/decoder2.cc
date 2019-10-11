#include "arith_code.cc"
#include <stdio.h>
#include "common.cc"
int decoder_input_callback(void *p)
{
  if( feof(stdin) ) {
    return -1;
  }
  int b;
  //change as you need
  b = fgetc(stdin);
  b = b - '0';
  return b;
}
int main()
{
  init();
  // decoder object
  AC::Decoder *D = new AC::Decoder(NULL, decoder_input_callback) ;
  // insert frequencies into Decoder
  D->cumulative_frequencies = cum_freq;
  D->max_symbol = n_symbols;
  int n=0, s;
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
