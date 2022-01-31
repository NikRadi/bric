#!/bin/bash
set +e # Exit if any statements return a non-true value
rm -f $OUT_FILE

TIME_BEFORE=`date +%T.%3N` # HH:MM:SS.MS
SIZE_BYTES=`wc -c < Main.c`
SIZE_NODES=$($AST_COUNTER Main.c)
$COMPILE_SCRIPT
RETURN_CODE=$?
TIME_AFTER=`date +%T.%3N`

echo "$TIME_BEFORE,$TIME_AFTER,$SIZE_BYTES,$SIZE_NODES,$RETURN_CODE" >> "$PREDICATE_CSV"
exit $RETURN_CODE
