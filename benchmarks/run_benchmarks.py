from dataclasses import dataclass
from typing import Iterable, Tuple
import os
import pandas as pd
import shutil
import subprocess
import time


@dataclass
class FileSize:
    test: str = ""
    num_bytes: int = -1 # 'bytes' is a class so we have to name it 'num_bytes'
    nodes: int = -1
    err_nodes: int = -1


MAX_SECONDS_TO_REDUCE = 60 * 60
FILESIZE_COLUMNS: Iterable[str] = [variable_name for variable_name in FileSize.__dataclass_fields__]
AST_COUNTER_EXE = os.path.realpath("astcounter")
BRIC_EXE = os.path.realpath("bric")
PERSES_EXE = os.path.realpath("perses_deploy.jar")
REDUCERS = {
    "creduce":    ["creduce", "PredicateWrapper.sh", "Main.c"],
    # "perses":     ["java", "-jar", PERSES_EXE, "--test-script", "PredicateWrapper.sh", "--input-file", "Main.c", "--in-place", "true"],
    # "bric-ddmin": [BRIC_EXE, "Main.c", "PredicateWrapper.sh", "-ddmin"],
    # "bric-hdd":   [BRIC_EXE, "Main.c", "PredicateWrapper.sh", "-hdd"],
    # "bric-br":    [BRIC_EXE, "Main.c", "PredicateWrapper.sh", "-br"],
    # "bric-gbr":   [BRIC_EXE, "Main.c", "PredicateWrapper.sh", "-gbr"]
}


def find_file_size(file_name: str) -> Tuple[int, int, int]:
    output = subprocess.run([AST_COUNTER_EXE, file_name], capture_output=True)
    output_str = output.stdout.decode().split(",")

    bytes = os.path.getsize(file_name)
    nodes = int(output_str[0])
    err_nodes = int(output_str[1])
    return bytes, nodes, err_nodes


if __name__ == "__main__":
    file_sizes = []
    environment = os.environ.copy()
    filepath = os.path.dirname(os.path.realpath(__file__))
    subdirs = sorted([d.path for d in os.scandir(filepath) if d.is_dir and d.name.startswith("clang-")])
    for subdir in subdirs:
        # Information about the file to reduce.
        main_path = os.path.join(subdir, "Main.c")
        subdir_name = os.path.split(subdir)[-1]
        b, n, e = find_file_size(main_path)
        file_size = FileSize(subdir_name, b, n, e)
        file_sizes.append(file_size)

    df = pd.DataFrame(file_sizes, columns=FILESIZE_COLUMNS)
    df.to_csv("file_sizes.csv", index=False)

    for subdir in subdirs:
        environment["COMPILE_SCRIPT"] = os.path.join(subdir, "Compile.sh")
        environment["OUT_FILE"] = os.path.join(subdir, "out.txt")

        # Perses requires that the test-file and predicate-file are in the
        # same folder, so we copy the predicate into the test folder.
        shutil.copy("PredicateWrapper.sh", subdir)
        os.chdir(subdir)

        for reducer_name in REDUCERS.keys():
            print(f"{subdir} {reducer_name}", end="", flush=True)
            output_csv = os.path.join(subdir, f"{reducer_name}_output.csv")
            environment["PREDICATE_CSV"] = output_csv

            dir_name = os.path.join(subdir, f"{reducer_name}_run")
            os.mkdir(dir_name)
            shutil.copy("Main.c", dir_name)
            shutil.copy("Compile.sh", dir_name)
            shutil.copy("PredicateWrapper.sh", dir_name)

            os.chdir(dir_name)
            is_terminated = False
            time_before = time.time()
            try:
                result = subprocess.run(
                    REDUCERS[reducer_name],
                    stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                    env=environment,
                    timeout=MAX_SECONDS_TO_REDUCE
                )
            except subprocess.TimeoutExpired:
                is_terminated = True

            time_after = time.time()
            print(f" ({round(time_after - time_before, 2)}s)", end="")
            os.chdir("..")
            if is_terminated:
                print(" terminated", end="")

            print()

        os.chdir("..")