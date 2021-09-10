#!/bin/sh
gcc Main.c -o Main.out
./Main.out | grep "Hi"
rm Main.out