from dataclasses import dataclass
import os
import shutil
import subprocess
import time


@dataclass
class Benchmark:
    tool_name : str
    test_name : str
    bytes_before : int = -1
    bytes_after : int = -1
    time_seconds : float = -1
    num_predicate_calls : int = -1
    notes : str = ""


if __name__ == "__main__":
    tools = [
        ("creduce", ["creduce", "Predicate.sh", "Main.c"]),
        ("perses", ["java", "-jar", "../perses_deploy.jar", "--test-script", "Predicate.sh", "--input-file", "Main.c", "--in-place", "true"])
    ]

    benchmarks = []
    environment = os.environ.copy()
    dir_path = os.path.dirname(os.path.realpath(__file__))
    sub_dirs = [f.path for f in os.scandir(dir_path) if f.is_dir()]
    sub_dirs.sort()
    print(f"running benchmarks in {dir_path}")
    for tool in tools:
        for sub_dir in sub_dirs:
            os.chdir(dir_path)
            sub_dir_name = os.path.split(sub_dir)[-1]
            benchmark = Benchmark(tool[0], sub_dir_name)
            if sub_dir_name[:-2] != "test":
                print(f"{sub_dir_name}: skip")
                benchmark.notes = "skipped"
                benchmarks.append(benchmark)
                continue

            os.chdir(sub_dir_name)
            if not os.path.isfile("Main.c"):
                print(f"{sub_dir_name}: missing Main.c")
                benchmark.notes = "missing Main.c"
                benchmarks.append(benchmark)
                continue

            # Copy 'Main.c' so that the reducer works on a copy of the file.
            # The tools typically rename the file, making it hard to clean up.
            shutil.copyfile("Main.c", "OriginalMain.c")

            print(f"running {sub_dir_name} ({tool[0]})")
            benchmark.bytes_before = os.path.getsize("Main.c")

            subprocess.run(["chmod", "u+x", "Predicate.sh"], stdout=subprocess.DEVNULL)
            subprocess.run(["dos2unix", "Predicate.sh"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            environment["PREDICATE_CSV"] = os.path.join(dir_path, sub_dir_name) + "/output.csv"
            time_before = time.time()
            result = subprocess.run(
                tool[1],
                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                env=environment
            )

            time_after = time.time()
            if result.returncode != 0:
                print(f"test in directory '{sub_dir_name}' returned exit code {result.returncode}")
                benchmark.notes = f"returned exit code {result.returncode}"
                benchmarks.append(benchmark)
                continue

            benchmark.time_seconds = time_after - time_before
            benchmark.bytes_after = os.path.getsize("Main.c")
            benchmark.num_predicate_calls = sum(1 for line in open("output.csv"))
            benchmarks.append(benchmark)

            # Clean up after the tool
            os.remove("output.csv")
            os.rename("OriginalMain.c", "Main.c")

    os.chdir(dir_path)
    filename = f"benchmark.csv"
    file = open(filename, "w")
    file.write("tool,test,bytes before,bytes after,time (s),predicate calls,notes\n")
    for b in benchmarks:
        file.write(f"{b.tool_name},{b.test_name},{b.bytes_before},{b.bytes_after},{b.time_seconds:.2f},{b.num_predicate_calls},{b.notes}\n")

    file.close()
