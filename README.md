# ArithCodeTut

## Arithmetic Coding tutorial code

This code implements the algorithm  described in

WITTEN, Ian H., NEAL, Radford M., CLEARY, John G. 
_Arithmetic coding for data compression._
Communications of the ACM, 1987, 30.6: 520-540.

It is written in C++ . It compiles fine with gcc 6.3

The library is all contained in the files `arith_code.cc`  `arith_code.hh`  `arith_code_config.hh`
(the latter contains some tunable parameters).

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


## Code

Currently, for simplicity, there is no "library".

The code for the arithmetic encoder/decoder is contained in the file `arith_code.cc`

It is then included in all other files as needed.

(This will include  arith_code.hh  arith_code_config.hh as well).

## Overview of the code

All classes, variables and functions  are documented using Doxygen.
[You can browse the documentation here](https://mennucc.github.io/ArithCodeTut/html/index.html)

## Examples

The program `arith_simple` encodes/decodes standard input to standard output.
(Note that, for simplicity, when encoding bits are printed as characters *0* and *1*, so that
the output is text).

If you wish to learn to use the library, you can start by reading the source code `arith_simple.cc`
 
The programs `arith_file_2` and  `arith_file`  can compress and decompress files.
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

## Testing the code

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
