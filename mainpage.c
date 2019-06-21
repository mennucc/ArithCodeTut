
/*! \mainpage The StiefelCurve library

  \section intro_sec Introduction

  This library implements  arithmetic coding compression.

  You may read 
  <a href="https://en.wikipedia.org/wiki/Arithmetic_coding">
  the wikipedia page on the argument to learn what this is about.</a>,

  The library is all contained in the file arith_code.cc

  The main objects in it  are the classes Encoder and  Decoder.

  The Encoder class encodes a sequence of symbols;
  each symbol is an integer number in the range 0...max_symbol-1 ;
  the Encoder produces an output sequence of bits.

  The Decoder class decodes the sequence of bits and recovers the sequence of symbols.

  This code is very versatile:

 <ul><li> it can encode sequences of symbols
  where each symbol has its own random distribution,
  and even "max_symbol" may change;
 <li> the output of the encoder/decoder can be processed
   using "callbacks" or by polling ("callbacks" are recommended, though)
 <li> the algorithm can work in reverse:
   the decoder can decode a random i.i.d. string of bits
   and convert it to a stream of symbols with desired distribution
  (which can also be re-encoded to the original string of bits)
 </ul>

  \section compile_sec Compiling

  The code was prepared to be compiled with gcc

  \section install_sec Using as a library

  The code for the arithmetic encoder/decoder is contained in the file arith_code.cc

  It is then included in all other files as needed.

  \section code_sec Overview of the code

  All classes, variables and functions  are documented, see the above menus.

  \section Examples

  The program arith_simple encodes/decodes standard input to standard output.
  If you wish to learn to use the library, you can start by reading the source code arith_simple.cc
 
  The programs arith_file_2.cc and  arith_file.cc  can compress and decompress files.

  \section code_sec Testing the code

  The program  arith_test.cc can test the code in many different ways.

 */
