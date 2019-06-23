# ArithCodeTut

## Arithmetic Coding tutorial code

This code implements the algorithm  described in

WITTEN, Ian H., NEAL, Radford M., CLEARY, John G. 
__Arithmetic coding for data compression.__
Communications of the ACM, 1987, 30.6: 520-540.

It is written in C++ . It compiles fine with gcc 6.3

The library is all contained in the `file arith_code.cc`

The main objects in it  are the classes Encoder and  Decoder.

The Encoder class encodes a sequence of symbols;
each symbol is an integer number in the range 
`0...max_symbol-1` ;
the Encoder produces an output sequence of bits.

The Decoder class decodes the sequence of bits and recovers the sequence of symbols.

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

The code for the arithmetic encoder/decoder is contained in the file arith_code.cc

It is then included in all other files as needed.

## Overview of the code

All classes, variables and functions  are documented, see the above menus.

## Examples

The program `arith_simple.cc` encodes/decodes standard input to standard output.

If you wish to learn to use the library, you can start by reading the source code `arith_simple.cc`
 
The programs `arith_file_2.cc` and  `arith_file.cc`  can compress and decompress files.

## Testing the code

The program  `test/arith_test.cc` can test the code in many different ways.

## Documentation

All relevant functions, variables and classes are documented

(Please see Doxygen documentations)
