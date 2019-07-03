// byte non ascii , value 136 at position 166669 
// byte non ascii , value 131 at position 303436 
// byte non ascii , value 131 at position 303555 
// byte non ascii , value 131 at position 303777 
// byte non ascii , value 131 at position 304840 
// byte non ascii , value 135 at position 441356 
// byte non ascii , value 138 at position 470071 
// byte non ascii , value 135 at position 691079 
// count of letters in silesia/dickens
int count_ascii[127] = {
            0 , // ascii 0
            0 , // ascii 1
            0 , // ascii 2
            0 , // ascii 3
            0 , // ascii 4
            0 , // ascii 5
            0 , // ascii 6
            0 , // ascii 7
            0 , // ascii 8
            1 , // ascii 9
       200783 , // ascii 10
            0 , // ascii 11
            0 , // ascii 12
       200783 , // ascii 13
            0 , // ascii 14
            0 , // ascii 15
            0 , // ascii 16
            0 , // ascii 17
            0 , // ascii 18
            0 , // ascii 19
            0 , // ascii 20
            0 , // ascii 21
            0 , // ascii 22
            0 , // ascii 23
            0 , // ascii 24
            0 , // ascii 25
            1 , // ascii 26
            0 , // ascii 27
            0 , // ascii 28
            0 , // ascii 29
            0 , // ascii 30
            0 , // ascii 31
      1739994 , // ascii 32 ' '
        12177 , // ascii 33 '!'
        17708 , // ascii 34 '"'
           27 , // ascii 35 '#'
           13 , // ascii 36 '$'
           53 , // ascii 37 '%'
           28 , // ascii 38 '&'
        70754 , // ascii 39 '''
         2585 , // ascii 40 '('
         2585 , // ascii 41 ')'
          968 , // ascii 42 '*'
            4 , // ascii 43 '+'
       173313 , // ascii 44 ','
        27028 , // ascii 45 '-'
        79379 , // ascii 46 '.'
           82 , // ascii 47 '/'
          565 , // ascii 48 '0'
          483 , // ascii 49 '1'
          269 , // ascii 50 '2'
          165 , // ascii 51 '3'
          134 , // ascii 52 '4'
          128 , // ascii 53 '5'
          125 , // ascii 54 '6'
          112 , // ascii 55 '7'
          133 , // ascii 56 '8'
          189 , // ascii 57 '9'
         2758 , // ascii 58 ':'
        16045 , // ascii 59 ';'
           13 , // ascii 60 '<'
           25 , // ascii 61 '='
           16 , // ascii 62 '>'
         8153 , // ascii 63 '?'
           56 , // ascii 64 '@'
        12536 , // ascii 65 'A'
         9762 , // ascii 66 'B'
        11099 , // ascii 67 'C'
         9577 , // ascii 68 'D'
         7989 , // ascii 69 'E'
         4935 , // ascii 70 'F'
         5639 , // ascii 71 'G'
        12120 , // ascii 72 'H'
        46072 , // ascii 73 'I'
         3741 , // ascii 74 'J'
         2485 , // ascii 75 'K'
         5720 , // ascii 76 'L'
        23384 , // ascii 77 'M'
         6801 , // ascii 78 'N'
         5758 , // ascii 79 'O'
         6984 , // ascii 80 'P'
          517 , // ascii 81 'Q'
         5577 , // ascii 82 'R'
        13895 , // ascii 83 'S'
        20791 , // ascii 84 'T'
         2162 , // ascii 85 'U'
         1425 , // ascii 86 'V'
        10565 , // ascii 87 'W'
          372 , // ascii 88 'X'
         4841 , // ascii 89 'Y'
           33 , // ascii 90 'Z'
          267 , // ascii 91 '['
            0 , // ascii 92 '\'
          271 , // ascii 93 ']'
            0 , // ascii 94 '^'
           14 , // ascii 95 '_'
          617 , // ascii 96 '`'
       600653 , // ascii 97 'a'
       108768 , // ascii 98 'b'
       169832 , // ascii 99 'c'
       345856 , // ascii 100 'd'
       923391 , // ascii 101 'e'
       159484 , // ascii 102 'f'
       158749 , // ascii 103 'g'
       474354 , // ascii 104 'h'
       484330 , // ascii 105 'i'
         6279 , // ascii 106 'j'
        62472 , // ascii 107 'k'
       287299 , // ascii 108 'l'
       188043 , // ascii 109 'm'
       529813 , // ascii 110 'n'
       582192 , // ascii 111 'o'
       123616 , // ascii 112 'p'
         7065 , // ascii 113 'q'
       446809 , // ascii 114 'r'
       462461 , // ascii 115 's'
       655603 , // ascii 116 't'
       211133 , // ascii 117 'u'
        66861 , // ascii 118 'v'
       172472 , // ascii 119 'w'
        11070 , // ascii 120 'x'
       158043 , // ascii 121 'y'
         2180 , // ascii 122 'z'
            2 , // ascii 123 '{'
            0 , // ascii 124 '|'
            2 , // ascii 125 '}'
           22 , // ascii 126 '~'
};

// total  10192446 bytes
// for lowercase     : probability 0.725913, count 7398828 (284570.3 on average)
// for uppercase     : probability 0.023035, count 234780 (9030.0 on average)
// for digits        : probability 0.000226, count 2303 ( 230.3 on average)
// for punctuation   : probability 0.040713, count 414965 (12967.7 on average)
// for space         : probability 0.170714, count 1739994 (1739994.0 on average)
// for line feed     : probability 0.019699, count 200783 (200783.0 on average)
// for carriage return: probability 0.019699, count 200783 (200783.0 on average)
// for non ascii        probability 0.000001, count 8 (   8.0 on average)
// other ascii chars count 2 , on average 0.012658 frequency
