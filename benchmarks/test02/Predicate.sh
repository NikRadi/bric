#!/bin/sh
gcc Main.c -o Main.out
./Main.out | grep "Hello"
rm Main.out