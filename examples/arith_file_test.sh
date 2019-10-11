#!/bin/sh

set -e

f="$1"

if ! test -f "$f" ; then
    echo DOES NOT EXISTS "$1"
    exit 2
fi

t=`tempfile`

o=`tempfile`

echo ====== ENCODE "$f"

./arith_file -C "$f" $t

echo ====== DECODE "$f"

./arith_file -D  $t $o

echo ====== 

if cmp "$f" $o ; then
    echo PERFECT RECONSTRUCTION
else
  ls -l "$f" $o
fi

rm "$o" "$t"

