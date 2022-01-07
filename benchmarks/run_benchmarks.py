from dataclasses import dataclass
from typing import Iterable, Tuple
import os
import pandas as pd
import shutil
import subprocess
import time


@dataclass
class FileSize:
    test: str
    num_bytes: int = -1 # 'bytes' is a class so we have to name it 'num_bytes'
    nodes: int = -1
    err_nodes: int = -1


# MAX_SECONDS_TO_REDUCE = 60 * 60
MAX_SECONDS_TO_REDUCE = 180
FILESIZE_COLUMNS: Iterable[str] = [variable_name for variable_name in FileSize.__dataclass_fields__]
REDUCERS = {
    # "creduce":      ["creduce", "PredicateWrapper.sh", "Main.c"],
    # "perses":       ["java", "-jar", "../perses_deploy.jar", "--test-script", "PredicateWrapper.sh", "--input-file", "Main.c", "--in-place", "true"],
    # "bric-ddmin":   ["../bric", "Main.c", "PredicateWrapper.sh", "-ddmin"],
    # "bric-hdd":     ["../bric", "Main.c", "PredicateWrapper.sh", "-hdd"],
    # "bric-br":      ["../bric", "Main.c", "PredicateWrapper.sh", "-br"],
    "bric-gbr":     ["../bric", "Main.c", "PredicateWrapper.sh", "-gbr"]
}


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
    file_sizes = []
    environment = os.environ.copy()
    filepath = os.path.dirname(os.path.realpath(__file__))
    subdirs = sorted([d.path for d in os.scandir(filepath) if d.is_dir and d.name.startswith("clang")])
    for subdir in subdirs:
        environment["COMPILE_SCRIPT"] = os.path.join(subdir, "Compile.sh")
        environment["OUT_FILE"] = os.path.join(subdir, "out.txt")

        # Perses requires that the test-file and predicate-file are in the
        # same folder, so we copy the predicate into the test folder.
        shutil.copy("PredicateWrapper.sh", subdir)
        os.chdir(subdir)

        # Information about the file to reduce.
        main_path = os.path.join(subdir, "Main.c")
        subdir_name = os.path.split(subdir)[-1]
        b, n, e = find_file_size(main_path)
        file_size = FileSize(subdir_name, b, n, e)
        file_sizes.append(file_size)

        # Have a copy of the original "Main.c". The tools may change "Main.c".
        shutil.copyfile("Main.c", "OriginalMain.c")
        for reducer_name in REDUCERS.keys():
            print(f"{subdir_name} {reducer_name}", end="")
            output_csv = os.path.join(subdir, f"{reducer_name}_output.csv")
            environment["PREDICATE_CSV"] = output_csv

            # Ensure that the size of the main file is the same for
            # every reducer.
            b, n, e = find_file_size(main_path)
            if b != file_size.num_bytes or n != file_size.nodes or e != file_size.err_nodes:
                print(f"ERROR: {main_path} size not matching")

            if os.path.isfile("Main.c"):
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
                if is_terminated:
                    print(" terminated", end="")

                print()
            else:
                print(f"ERROR: missing {output_csv}")

            shutil.copyfile("OriginalMain.c", "Main.c")

        # Clean up folder. The "output.csv" has already been removed.
        try_remove_file("a.out")
        try_remove_file("PredicateWrapper.sh")
        os.remove("OriginalMain.c")
        for file in os.listdir(subdir):
            if file.endswith(".orig"):
                os.remove(file)

        os.chdir("..")

    df = pd.DataFrame(file_sizes, columns=FILESIZE_COLUMNS)
    df.to_csv("file_sizes.csv", index=False)