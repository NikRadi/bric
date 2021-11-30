from dataclasses import dataclass
from typing import Iterable, Tuple
import os
import pandas as pd
import shutil
import subprocess
import time


@dataclass
class Benchmark:
    test: str
    tool: str
    bytes_before: int = -1
    bytes_after: int = -1
    nodes_before: int = -1
    nodes_after: int = -1
    err_nodes_before: int = -1
    err_nodes_after: int = -1
    time_sec: float = -1
    predicate_calls: int = -1
    failed_compiles: int = -1
    failed_runs: int = -1
    other: str = ""


COLUMNS: Iterable[str] = [variable_name for variable_name in Benchmark.__dataclass_fields__]
TOOLS: Tuple[str, Iterable[str]] = [
    ("creduce", ["creduce", "PredicateWrapper.sh", "Main.c"]),
    ("perses", ["java", "-jar", "../perses_deploy.jar", "--test-script", "PredicateWrapper.sh", "--input-file", "Main.c", "--in-place", "true"]),
    ("bric-ddmin", ["../bric", "Main.c", "PredicateWrapper.sh", "-ddmin"]),
    ("bric-hdd", ["../bric", "Main.c", "PredicateWrapper.sh", "-hdd"]),
    ("bric-br", ["../bric", "Main.c", "PredicateWrapper.sh", "-br"]),
    ("bric-gbr", ["../bric", "Main.c", "PredicateWrapper.sh", "-gbr"]),
]


def find_file_size(file_name: str) -> Tuple[int, int, int]:
    output = subprocess.run(["../astcounter", file_name], capture_output=True)
    output_str = output.stdout.decode().split(",")

    bytes = os.path.getsize(file_name)
    nodes = int(output_str[0])
    err_nodes = int(output_str[1])
    return bytes, nodes, err_nodes


def try_remove_file(file_name: str) -> None:
    if os.path.isfile(file_name):
        os.remove(file_name)


if __name__ == "__main__":
    benchmarks = []
    environment = os.environ.copy()
    filepath = os.path.dirname(os.path.realpath(__file__))
    subdirs = sorted([d.path for d in os.scandir(filepath) if d.is_dir])
    for subdir in subdirs:
        subdir_name = os.path.split(subdir)[-1]
        if not subdir_name.startswith("test"):
            continue

        # Prepare the environment variables
        environment["PREDICATE_CSV"] = os.path.join(subdir, "output.csv")
        environment["PREDICATE_NAME"] = os.path.join(subdir, "Predicate.sh")

        # Perses requires that the test-file and predicate-file are in the
        # same folder, so we copy the predicate into the test folder.
        shutil.copy("PredicateWrapper.sh", subdir)

        # Ensure that we have permission to run the predicate and that the
        # predicate has unix line endings.
        os.chdir(subdir)
        subprocess.run(["chmod", "u+x", "Predicate.sh"], stdout=subprocess.DEVNULL)
        subprocess.run(["dos2unix", "Predicate.sh"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

        # Have a copy of the original "Main.c". The tools may change "Main.c".
        shutil.copyfile("Main.c", "OriginalMain.c")
        for tool in TOOLS:
            print(f"{subdir_name} {tool[0]}")
            main_path = os.path.join(subdir, "Main.c")
            benchmark = Benchmark(subdir_name, tool[0])
            shutil.copyfile("OriginalMain.c", "Main.c")

            # Finding the size of the "Main.c" file could be done outside
            # of this loop. However, we include it to ensure all tools
            # reduce a file of the exact same size.
            b, n, e = find_file_size(main_path)
            benchmark.bytes_before = b
            benchmark.nodes_before = n
            benchmark.err_nodes_before = e
            if os.path.isfile("Main.c"):
                time_before = time.time()
                result = subprocess.run(
                    tool[1],
                    stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                    env=environment
                )

                time_after = time.time()
                benchmark.time_sec = round(time_after - time_before, 2)

                b, n, e = find_file_size( main_path)
                benchmark.bytes_after = b
                benchmark.nodes_after = n
                benchmark.err_nodes_after = e
                if os.path.isfile("output.csv"):
                    output = pd.read_csv("output.csv", header=None)
                    benchmark.predicate_calls = len(output.index)
                    benchmark.failed_compiles = len(output.loc[output[3] == 3].values)
                    benchmark.failed_runs = len((output.loc[(output[3] == 0) & (output[4] == 1)].values))
                    os.remove("output.csv")
                else:
                    print("missing output.csv")
                    benchmark.other += "Missing output.csv. "
            else:
                print("missing output.csv")
                benchmark.other += "Missing Main.c. "

            benchmarks.append(list(benchmark.__dict__.values()))

        # Clean up folder. The "output.csv" has already been removed.
        try_remove_file("a.out")
        try_remove_file("PredicateWrapper.sh")
        os.remove("OriginalMain.c")
        for file in os.listdir(subdir):
            if file.endswith(".orig"):
                os.remove(file)

        os.chdir("..")

    df = pd.DataFrame(benchmarks, columns=COLUMNS)
    df.to_csv("benchmarks.csv", index=False)