export AST_COUNTER=$PWD/astcounter
for dir in clang-*
do
    cp PredicateWrapper.sh $dir
    cd $dir
    rm -f p.csv
    export COMPILE_SCRIPT=$PWD/Compile.sh
    export OUT_FILE=$PWD/o.txt
    export PREDICATE_CSV=$PWD/p.csv
    TIME_BEFORE= # HH:MM:SS.MS
    date +%T.%3N > info.txt
    # timelimit -t1800 creduce PredicateWrapper.sh Main.c
    # timelimit -t1800 java -Xmx8192m -jar ../perses_deploy.jar --test-script PredicateWrapper.sh --input-file Main.c --in-place true
    # timelimit -t1800 ../bric Main.c PredicateWrapper.sh
    timelimit -t1800 ../bric Main.c PredicateWrapper.sh;creduce PredicateWrapper.sh Main.c
    date +%T.%3N >> info.txt
    cd ..
done