// count of letters in silesia/webster
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
            0 , // ascii 9
       930838 , // ascii 10
            0 , // ascii 11
            0 , // ascii 12
       930838 , // ascii 13
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
      3854483 , // ascii 32 ' '
         4620 , // ascii 33 '!'
       137507 , // ascii 34 '"'
         2818 , // ascii 35 '#'
           61 , // ascii 36 '$'
           35 , // ascii 37 '%'
       102563 , // ascii 38 '&'
        13339 , // ascii 39 '''
       183584 , // ascii 40 '('
       183565 , // ascii 41 ')'
       115806 , // ascii 42 '*'
        16301 , // ascii 43 '+'
       477252 , // ascii 44 ','
       108820 , // ascii 45 '-'
       956114 , // ascii 46 '.'
      1496831 , // ascii 47 '/'
         3720 , // ascii 48 '0'
        33168 , // ascii 49 '1'
        28728 , // ascii 50 '2'
        13042 , // ascii 51 '3'
         7430 , // ascii 52 '4'
         5596 , // ascii 53 '5'
         4360 , // ascii 54 '6'
         4390 , // ascii 55 '7'
         3050 , // ascii 56 '8'
         2656 , // ascii 57 '9'
         8528 , // ascii 58 ':'
       292861 , // ascii 59 ';'
      3011880 , // ascii 60 '<'
          930 , // ascii 61 '='
      3011945 , // ascii 62 '>'
       118673 , // ascii 63 '?'
           68 , // ascii 64 '@'
        95497 , // ascii 65 'A'
        55325 , // ascii 66 'B'
        62461 , // ascii 67 'C'
        29042 , // ascii 68 'D'
        33437 , // ascii 69 'E'
        43762 , // ascii 70 'F'
        35058 , // ascii 71 'G'
        30672 , // ascii 72 'H'
        38803 , // ascii 73 'I'
         8655 , // ascii 74 'J'
         3978 , // ascii 75 'K'
        49522 , // ascii 76 'L'
        38073 , // ascii 77 'M'
        18120 , // ascii 78 'N'
        53543 , // ascii 79 'O'
        43382 , // ascii 80 'P'
         2421 , // ascii 81 'Q'
        42065 , // ascii 82 'R'
       114653 , // ascii 83 'S'
       100984 , // ascii 84 'T'
         9299 , // ascii 85 'U'
         6238 , // ascii 86 'V'
        22553 , // ascii 87 'W'
          479 , // ascii 88 'X'
         2412 , // ascii 89 'Y'
        11552 , // ascii 90 'Z'
        96514 , // ascii 91 '['
           33 , // ascii 92 '\'
        96540 , // ascii 93 ']'
          614 , // ascii 94 '^'
           11 , // ascii 95 '_'
        41708 , // ascii 96 '`'
      1567271 , // ascii 97 'a'
       590299 , // ascii 98 'b'
       847754 , // ascii 99 'c'
      1029581 , // ascii 100 'd'
      2621499 , // ascii 101 'e'
       815081 , // ascii 102 'f'
       412332 , // ascii 103 'g'
       976579 , // ascii 104 'h'
      2419415 , // ascii 105 'i'
        20726 , // ascii 106 'j'
       222441 , // ascii 107 'k'
       997917 , // ascii 108 'l'
       467534 , // ascii 109 'm'
      1516975 , // ascii 110 'n'
      2052474 , // ascii 111 'o'
      1178240 , // ascii 112 'p'
       106152 , // ascii 113 'q'
      1353292 , // ascii 114 'r'
      1576983 , // ascii 115 's'
      1576393 , // ascii 116 't'
       765471 , // ascii 117 'u'
       216223 , // ascii 118 'v'
       494538 , // ascii 119 'w'
        47716 , // ascii 120 'x'
       289595 , // ascii 121 'y'
        19795 , // ascii 122 'z'
         3832 , // ascii 123 '{'
        14926 , // ascii 124 '|'
         3836 , // ascii 125 '}'
           26 , // ascii 126 '~'
};

// total  41458703 bytes
// for lowercase     : probability 0.583286, count 24182276 (930087.5 on average)
// for uppercase     : probability 0.022962, count 951986 (36614.8 on average)
// for digits        : probability 0.002560, count 106140 (10614.0 on average)
// for punctuation   : probability 0.253316, count 10502141 (328191.9 on average)
// for space         : probability 0.092972, count 3854483 (3854483.0 on average)
// for line feed     : probability 0.022452, count 930838 (930838.0 on average)
// for carriage return: probability 0.022452, count 930838 (930838.0 on average)
// for non ascii        probability 0.000000, count 0 (   0.0 on average)
// other ascii chars count 1 , on average 0.006329 frequency
