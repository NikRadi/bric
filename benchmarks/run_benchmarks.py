import datetime
import os
import subprocess


class Benchmark:
    def __init__(self, test_dir_name, lines_before, lines_after):
        self.test_dir_name = test_dir_name
        self.lines_before = lines_before
        self.lines_after = lines_after


def count_file_lines(file_name):
    return sum(1 for _ in open(file_name))


if __name__ == "__main__":
    benchmarks = []
    dir_path = os.path.dirname(os.path.realpath(__file__))
    sub_dirs = [f.path for f in os.scandir(dir_path) if f.is_dir()]
    print(f"starting in dir {dir_path}")
    for sub_dir in sub_dirs:
        os.chdir(dir_path)
        sub_dir_name = os.path.split(sub_dir)[-1]
        if sub_dir_name[:-2] != "test":
            print(f"skipping directory '{sub_dir_name}'")
            continue

        print(f"testing {sub_dir_name}")
        os.chdir(sub_dir_name)
        num_lines_before = count_file_lines("Main.c")

        result = subprocess.run([f"creduce", "./Predicate.sh", "Main.c"], stdout=subprocess.PIPE)
        if result.returncode != 0:
            print(f"test in directory '{sub_dir_name}' returned exit code {result.returncode}")
            continue

        num_lines_after = count_file_lines("Main.c")
        os.remove("Main.c")
        os.rename("Main.c.orig", "Main.c")

        benchmark = Benchmark(sub_dir_name, num_lines_before, num_lines_after)
        benchmarks.append(benchmark)

    os.chdir(dir_path)
    now = datetime.datetime.now()
    filename = f"{now.year}{now.month}{now.day}{now.hour}{now.minute}{now.second}.csv"
    file = open(filename, "w")
    file.write("test name,lines before, lines after\n")
    for benchmark in benchmarks:
        file.write(f"{benchmark.test_dir_name},{benchmark.lines_before},{benchmark.lines_after}\n")

    file.close()
