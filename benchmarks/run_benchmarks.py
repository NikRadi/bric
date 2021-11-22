import os
import subprocess
import time


class Benchmark:
    def __init__(self):
        self.tool_name           = None
        self.test_name           = None
        self.bytes_before        = None
        self.bytes_after         = None
        self.time_seconds        = None
        self.num_predicate_calls = None


if __name__ == "__main__":
    benchmarks = []
    dir_path = os.path.dirname(os.path.realpath(__file__))
    sub_dirs = [f.path for f in os.scandir(dir_path) if f.is_dir()]
    print(f"running benchmarks in {dir_path}")
    for sub_dir in sub_dirs:
        os.chdir(dir_path)
        sub_dir_name = os.path.split(sub_dir)[-1]
        if sub_dir_name[:-2] != "test":
            print(f"skipping directory '{sub_dir_name}'")
            continue

        print(f"testing {sub_dir_name}")
        os.chdir(sub_dir_name)
        bytes_before = os.path.getsize("Main.c")

        time_before = time.time()
        result = subprocess.run([f"creduce", "./Predicate.sh", "Main.c"], stdout=subprocess.PIPE)
        time_after = time.time()
        if result.returncode != 0:
            print(f"test in directory '{sub_dir_name}' returned exit code {result.returncode}")
            continue

        bytes_after = os.path.getsize("Main.c")
        counter_file = open("counter.txt", "r")
        num_predicate_calls = len(counter_file.read())
        counter_file.close()

        os.remove("Main.c")
        os.rename("Main.c.orig", "Main.c")

        benchmark = Benchmark()
        benchmark.tool_name = "creduce"
        benchmark.test_name = sub_dir_name
        benchmark.bytes_before = bytes_before
        benchmark.bytes_after = bytes_after
        benchmark.time_seconds = time_before - time_after
        benchmark.num_predicate_calls = num_predicate_calls
        benchmarks.append(benchmark)

    os.chdir(dir_path)
    filename = f"benchmark.csv"
    assert not os.path.isfile(filename)
    file = open(filename, "w")
    file.write("tool name,test name,bytes before,bytes after,time seconds,num predicate calls\n")
    for b in benchmarks:
        file.write(f"{b.tool_name},{b.test_name},{b.bytes_before},{b.bytes_after},{b.time_seconds}{b.num_predicate_calls}\n")

    file.close()
