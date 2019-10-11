# ArithCodeTut

## Arithmetic Coding tutorial code

This code implements the algorithm  described in

WITTEN, Ian H., NEAL, Radford M., CLEARY, John G. 
_Arithmetic coding for data compression._
Communications of the ACM, 1987, 30.6: 520-540.

You may also want to read
[the wikipedia page on the argument](https://en.wikipedia.org/wiki/Arithmetic_coding)


## Getting the code

You can get the (latest version of) code from  https://github.com/mennucc/ArithCodeTut

## Code overview

The code is written in C++ . It compiles fine with gcc. (Versions 6.3 and 8.3 were used).

The library is all contained in the files `arith_code.cc` , `arith_code.hh` , `arith_code_config.hh`
(the latter contains some tunable parameters).

All classes, variables and functions  are documented using Doxygen.
[You can browse the documentation here](https://mennucc.github.io/ArithCodeTut/html/index.html)

The main objects in it  are the classes AC::Encoder and  AC::Decoder.

The AC::Encoder class encodes a sequence of symbols;
each symbol is an integer number in the range 
`0...max_symbol-1` ;
the Encoder produces an output sequence of bits.

The AC::Decoder class decodes the sequence of bits and recovers the sequence of symbols.

This code is very versatile:

- it can encode sequences of symbols
  where each symbol has its own random distribution,
  and even "max_symbol" may change;

- the output of the encoder/decoder can be processed
   using "callbacks" or by polling ("callbacks" are recommended, though)

- the algorithm can work in reverse:
   the decoder can decode a random i.i.d. string of bits
   and convert it to a stream of symbols with desired distribution
  (which can also be re-encoded to the original string of bits)

## Library

Currently, for simplicity, there is no "library".

The code for the arithmetic encoder/decoder is contained in the file `arith_code.cc`

It is then included in all other files as needed.

(This will include  arith_code.hh  arith_code_config.hh as well).

## Example usage

All these code snippets are available in the directory `snippet` for your convenience.


First we need some common code (saved as `common.cc`)

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

This simple program encodes an input string into bits, that are printed on the output (as chars '0' and '1').

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


This decodes that output and prints the symbols.

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

Further examples are in the aforementioned directory.

## More complex examples

Other examples are in the `examples` subdirectory.

The program `arith_simple` encodes/decodes standard input to standard output.
(Note that, for simplicity, when encoding bits are printed as characters *0* and *1*, so that
the output is text). It learns the probability distribution of the input symbol.

The program `arith_simple_markov` is similar, but it uses a more sophisticated
Markov chain model that learns the probability distribution of pairs of input symbols.

The programs  `arith_file_3`, `arith_file_2` and  `arith_file`  can compress and decompress files.
(Again, for simplicity, when encoding bits are saved as characters *0* and *1*, so that
the output is a text file).

## Flushing

The code implements a non-destructive flushing method, described in `doc/on_deflushing.pdf`

## Learning _arithmetic encoding_

This code can show how _arithmetic encoding_ works

The program `arith_simple` can be used to encode/decode strings.
(Its frequency table is initialized with frequencies of English letters).

For example,
```
echo -n hello | ./arith_simple -C
```
will encode the word `hello` (and the EOF symbol) using 32 bits,
that is, 5.3 bits for each input byte; it will print on stdout `01110111101011001010001111000100`.

At the same time
```
echo -n hello | ./arith_simple_v -C > /dev/null
```
will list all steps of encoding.

Then
```
echo 01110111101011001010001111000100 | ./arith_simple -D
```
will decode the message; and 
```
echo 01110111101011001010001111000100 | ./arith_simple_v -D > /dev/null
```
will show all steps of the decoder.

## Testing the library

To this end, `cd test` and `make` : this will build test programs.

Then `./arith_test_c_v 100 -U 3` will encode 100 symbols randomly chosen between {0,1,2} ;
and send the bits to the decoder.
It will print a verbose, colored output where you can follow the operation step by step.

To better read the printout, use `./arith_test_c_v 100 -U 3 | less -R` 

(If you define `AC_representation_bitsize` to  20 bits, then the extremes of the intervals will be printed
in binary format, and this is more instructive)

You can run the above with different choices of random input, and with/without periodic flushing.

For large number of symbols, you may use `./arith_test_c` that is less verbose.

`./arith_test_c 1000000 -p 137 -R 357` will feed one million
symbols in the encoder; each in the range {0...356} ; all equally distributed but with a  distribution chosen at random;
every 137 symbols the encoder will be flushed, and the decoder will _deflush_.


The code  `test/arith_test.cc` can stress test the code in many different ways. Use `cd test` then `make testall` to run some tests. The program `arith_test_c` tests the code using callbacks, whereas `arith_test_p` uses polling.

The program `arith_test_c_v` and`arith_test_p_v` are identical, just much more verbose.

## TODO: Python interface

If anyone needs a Python interface, please ask.
