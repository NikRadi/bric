#
# compiler error: run-time
#


clang-3.6.0 -O1 -m64 Main.c
RETURN_CODE=$?
if [ $RETURN_CODE != 0 ] ; then
    exit 1
fi

./a.out >$OUT_FILE 2>&1
RETURN_CODE=$?
if [ $RETURN_CODE != 136 ] ; then
    exit 1
fi

exit 0
