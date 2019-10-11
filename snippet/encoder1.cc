#include "arith_code.cc"
#include <stdio.h>
#include "common.cc"
void encoder_output_callback(int b, void *p)
{
  fputc(b+'0', stdout);
}
int main()
{
  init();
  // encoder object
  AC::Encoder *E = new AC::Encoder(encoder_output_callback) ;
  // character
  int c;
  while(EOF != (c = fgetc(stdin) ) ) {
    int s = char_to_symbol(c);
    if (s < 0 || s>= n_symbols )
      fprintf(stderr, "encoder ignored input char 0x%02x\n", c);
    else
      // encode symbol
      E->input_symbol(s,cum_freq);
  }
  E->input_symbol(eof_symbol, cum_freq);
  E->flush();
  // print statistics
  unsigned long ns = E->number_input_symbols(), nb = E->number_output_bits();
  fprintf(stderr," input symbols %lu (including eof, excluding flushing); output bits %lu,\n ratio %f bits per symbol\n",
	  ns,nb,(double)nb/(double)ns);
}
