# bric (Binary Reduction In C)
## An input reduction tool

### Algorithms
- multi-layer binary reduction
- single-layer binary reduction
- delta debugging (ddmin)
- hierarchical delta debugging (hdd)

### Benchmarks
In the benchmarks folder we have 10 sub-folders, each with a Compile.sh and Main.c. The Main.c files are from another input reduction tool, [perses](https://github.com/perses-project/perses/tree/master/benchmark). The Compile.sh files correspond to the r.sh files found in the perses project but abbreviated. The perses project has a GNU licence, which bric also has.  
There are three types of benchmark files. There are ones that produce compile time errors, ones that produce run time errors, and ones that produce wrong results. The benchmarks that produce wrong results have a correct_result.txt containing, as the name suggests, the correct result. The result can be obtained from a compiler without the particular bug that is being tested in the benchmark.
