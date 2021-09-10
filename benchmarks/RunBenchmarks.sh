#!/bin/sh
cd benchmarks/
for DIR in test*/
do
    cd $DIR
    creduce ./Predicate.sh Main.c
    cd ..
done