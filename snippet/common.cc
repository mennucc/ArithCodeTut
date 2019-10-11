// encode any byte
const int n_symbols =  257,
// and EOF
  eof_symbol = n_symbols - 1;
// map symbol to printable char
#define symbol_to_char(s) ((s>=' '&&s<='~')?s:'?')
// map char to symbol
#define char_to_symbol(c) c
// initialize  the frequencies, the comulative frequencies
AC::F_t *freq = NULL,  *cum_freq = NULL;
void init()
{
  freq = new AC::F_t[n_symbols];
  // you must initialize the frequencies
  for(int j=0; j<n_symbols; j++ ) {
    int c = symbol_to_char(j);
    // we decide that lowercase letters are more probable
    freq[j] = ( c >= 'a' &&   c <= 'z' ) ? 10 : 1 ;
  }
  // and we initialize the table of comulative frequencies
  cum_freq = new AC::F_t[n_symbols+1];
  AC::freq2cum_freq(cum_freq,freq,  n_symbols);
}
