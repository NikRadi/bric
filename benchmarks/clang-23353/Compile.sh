#
# compiler error: compile-time
#


clang-3.6.0 -O3 -m64 Main.c >$OUT_FILE 2>&1
grep "PLEASE ATTACH THE FOLLOWING FILES" $OUT_FILE
