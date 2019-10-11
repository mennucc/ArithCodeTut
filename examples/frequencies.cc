
void initialize_frequencies_nonnull(AC::F_t * freq)
{
  // no frequency can be zero
  for(int j=0;j<n_symbols;j++)
    freq[j]=1;
}


#include "corpus/count_dickens.c"
void initialize_frequencies_dickens(AC::F_t * freq)
{
  for(int j=0;j<126;j++) {
    freq[j]=count_ascii[j];
  }
  // set EOF as probable as '.'
  freq[256] = freq['.'];
  // no frequency can be zero
  for(int j=0;j<n_symbols;j++) {
    freq[j] += 1;
  }
  AC::renormalize_frequencies(freq,n_symbols);
}
