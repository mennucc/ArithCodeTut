#!/usr/bin/python

import sys, string

if len(sys.argv) <= 1:
    print('Provide input file , "-" for stdin')
    sys.exit(1)


if  sys.argv[1] != '-':
    fil=open(sys.argv[1])
else:
    fil=sys.stdin

n_symbols = 0x7F

extras_symbol = n_symbols

freq=[0] * (extras_symbol+1)

pos = 0
for a in fil:
    for l in a:
        pos += 1
        j = ord(l)
        if j < n_symbols:
            freq[j] += 1
        else:
            print('// byte non ascii , value %d at position %d ' %  (j, pos) )
            freq[extras_symbol] += 1

print("// count of letters in " + sys.argv[1])

print("int count_ascii[%d] = {" % n_symbols)

for (n,f) in enumerate(freq):
    if n < n_symbols:
        print( (" %s , // ascii %d" %   ( str(f).rjust(12), n )) +
                (   (" '%c'" % n) if n>=32 and n <= 126 else '' ))
print('};\n')

t = sum(freq)
#remainder of frequencies
rt = t
#remainder of symbols
rs = 256

print("// total  %d bytes" % t)
for subset,name in (( string.lowercase, 'lowercase'),
                        ( string.uppercase, 'uppercase'),
                        ( string.digits, 'digits'),
                        ( string.punctuation, 'punctuation'),
                        ( ' ', 'space'),
                        ( '\n', 'line feed'),
                        ( '\r', 'carriage return'),
                        ):
    l = sum(freq[ord(j)] for j in subset)
    print("// for %s: probability %f, count %d (%6.1f on average)" % (name.ljust(14), float(l)/float(t), l, float(l) / float(len(subset)) ) )
    rt = rt - l
    rs = rs - len(subset)

l = freq[extras_symbol]
name='non ascii'
print("// for %s  probability %f, count %d (%6.1f on average)" % (name.ljust(15), float(l)/float(t), l, float(l) / float(len(subset)) ) )
rt = rt - l
rs = rs - 1

print("// other ascii chars count %d , on average %f frequency" %  ( rt, float(rt) / float(rs) ) )
