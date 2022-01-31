#
# compiler error: wrong result
#


rm -f a.out
rm -f clang.out

# First, compile with clang-7-1.0 and run
CLANG_OUT_FILE="${OUT_FILE}clang.txt"
clang-7.1.0 -m64 -O0 -Wall -fwrapv -ftrapv -fsanitize=undefined -o clang.out Main.c
RETURN_CODE=$?
if [ $RETURN_CODE != 0 ] ; then
    exit 1
fi

./clang.out >$CLANG_OUT_FILE 2>&1
RETURN_CODE=$?
if [ $RETURN_CODE != 0 ] ; then
    exit 1
fi

if grep -q "runtime error" $CLANG_OUT_FILE ; then
    exit 1
fi

# Compile with the buggy compiler
clang-3.8.0 -Os -m32 Main.c
RETURN_CODE=$?
if [ $RETURN_CODE != 0 ] ; then
    exit 1
fi

# Run the compiled file
./a.out >$OUT_FILE 2>&1
RETURN_CODE=$?
if [ $RETURN_CODE != 0 ] ; then
    exit 1
fi

# Compare the result with clang
if diff -q $OUT_FILE $CLANG_OUT_FILE >/dev/null ; then
    exit 1
fi

exit 0
