#! /usr/bin/env bash
set +e

BEFORE=`date +%n`
COUNT=`wc -c < Main.c`
gcc Main.c
COMPILES="$?"
if [ $COMPILES -eq 0 ]
then
    ./a.out | grep "Hi"
    RUNS="$?"
else
    RUNS="1"
fi
AFTER=`date +%n`

echo "$BEFORE,$AFTER,$COUNT,$COMPILES,$RUNS" >> "$PREDICATE_CSV"
exit $RUNS