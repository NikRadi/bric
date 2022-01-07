#!/bin/bash
set +e # Exit if any statements return a non-true value

TIME_BEFORE=`date +%T.%3N` # HH:MM:SS.MS
SIZE_BYTES=`wc -c < Main.c`
$COMPILE_SCRIPT
grep -e "internal compiler error" -e "PLEASE ATTACH THE FOLLOWING FILES" $OUT_FILE
RETURN_CODE="$?"
TIME_AFTER=`date +%T.%3N`

echo "$TIME_BEFORE,$TIME_AFTER,$SIZE_BYTES,$RETURN_CODE" >> "$PREDICATE_CSV"
exit $RETURN_CODE