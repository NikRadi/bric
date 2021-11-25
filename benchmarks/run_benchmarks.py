from dataclasses import dataclass
import os
import subprocess
import time


@dataclass
class Benchmark:
    tool_name : str
    test_name : str
    bytes_before : int
    bytes_after : int
    time_seconds : float
    num_predicate_calls : int


if __name__ == "__main__":
    benchmarks = []
    environment = os.environ.copy()
    dir_path = os.path.dirname(os.path.realpath(__file__))
    sub_dirs = [f.path for f in os.scandir(dir_path) if f.is_dir()]
    sub_dirs.sort()
    print(f"running benchmarks in {dir_path}")
    for sub_dir in sub_dirs:
        os.chdir(dir_path)
        sub_dir_name = os.path.split(sub_dir)[-1]
        if sub_dir_name[:-2] != "test":
            print(f"{sub_dir_name}: skip")
            continue

        os.chdir(sub_dir_name)
        if not os.path.isfile("Main.c"):
            print(f"{sub_dir_name}: missing Main.c")
            continue

        print(f"running {sub_dir_name}")
        bytes_before = os.path.getsize("Main.c")

        subprocess.run(["chmod", "u+x", "Predicate.sh"], stdout=subprocess.DEVNULL)
        subprocess.run(["dos2unix", "Predicate.sh"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        environment["PREDICATE_CSV"] = os.path.join(dir_path, sub_dir_name) + "/output.csv"
        time_before = time.time()
        result = subprocess.run(
            ["creduce", "Predicate.sh", "Main.c"],
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
            env=environment
        )

        time_after = time.time()
        if result.returncode != 0:
            print(f"test in directory '{sub_dir_name}' returned exit code {result.returncode}")
            continue

        bytes_after = os.path.getsize("Main.c")
        num_predicate_calls = sum(1 for line in open("output.csv"))
        benchmark = Benchmark(
            tool_name           = "creduce",
            test_name           = sub_dir_name,
            bytes_before        = bytes_before,
            bytes_after         = bytes_after,
            time_seconds        = time_after - time_before,
            num_predicate_calls = num_predicate_calls
        )

        benchmarks.append(benchmark)

    os.chdir(dir_path)
    filename = f"benchmark.csv"
    file = open(filename, "w")
    file.write("tool,test,bytes before,bytes after,time (s),predicate calls\n")
    for b in benchmarks:
        file.write(f"{b.tool_name},{b.test_name},{b.bytes_before},{b.bytes_after},{b.time_seconds:.2f},{b.num_predicate_calls}\n")

    file.close()
