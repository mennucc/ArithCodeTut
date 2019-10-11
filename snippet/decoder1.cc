#include "arith_code.cc"
#include <stdio.h>
#include "common.cc"
void decoder_output_callback(int s, void *p)
{
  static int n=0;
  if( s == eof_symbol)
    fprintf(stdout,"decoder received EOF symbols\n");
  else {
    int c = symbol_to_char(s);
    fprintf(stdout,"decoder received symbol[%d] = %d , i.e. character '%c' \n", n, s, c);
    n++;
  }
}
int decoder_input_callback(void *p)
{
  if( feof(stdin) ) {
    // this will end the run
    return -1;
  }
  int  b = fgetc(stdin);
  b = b - '0';
  return b;
}
int main()
{
  init();
  // decoder object
  AC::Decoder *D = new AC::Decoder(decoder_output_callback, decoder_input_callback) ;
  D->cumulative_frequencies = cum_freq;
  D->max_symbol = n_symbols;
  D->run();
}
