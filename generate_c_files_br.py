from dataclasses import dataclass
from datetime import datetime
from random import randint, sample, choice
import os


@dataclass
class Function:
    identifier: str = None
    return_type: str = None
    statements = []


NUM_FUNCTIONS = [14, 18, 22, 26]
NUM_RELEVANTS = [4, 8, 12]


def randbool() -> bool:
    return bool(randint(0, 1))

if __name__ == "__main__":
    date = datetime.today().strftime("%y%m")
    dir_name = f"c_files_{date}"
    if not os.path.exists(dir_name):
        os.makedirs(dir_name)

    # Also generates a file for running all the tests
    run_tests_name = "run.bat"
    run_tests_f = os.path.join(dir_name, run_tests_name)
    open(run_tests_f, 'w').close() # Reset the file
    for num_functions in NUM_FUNCTIONS:
        for num_relevants in NUM_RELEVANTS:
            file_name = f"c_{num_functions}_{num_relevants}"
            test_name = f"t_{num_functions}_{num_relevants}.bat"
            with open(run_tests_f, "a") as f:
                f.write(f"..\\bric {file_name}.c {test_name}\n")

            # Generate test file
            expected_output = 'a' * num_relevants
            content =  f"cl {file_name}.c && "
            content += f"{file_name}.exe > out.txt && "
            content += f"findstr \"{expected_output}\" out.txt\n"
            f = os.path.join(dir_name, test_name)
            with open(f, "w") as test_file:
                test_file.write(content)

            # Generate C file
            # Subtract 1 because main must be relevant
            relevant_functions = sample(range(0, num_functions - 1), num_relevants - 1)
            function_ids = [i for i in range(num_functions - 1) if i not in relevant_functions]
            functions = []
            # Generate functions
            for i in range(num_functions - 1):
                f = Function()
                f.identifier = f"f{i}"
                if i in relevant_functions:
                    f.statements = ["printf(\"a\");"]

                functions.append(f)

            f = Function()
            f.identifier = "main"
            f.return_type = "int"
            f.statements = [f"f{i}();" for i in relevant_functions]
            f.statements.append("printf(\"a\");")
            functions.append(f)

            program = ""

            # Make function prototypes so all functions can be called
            for f in functions:
                if f.identifier == "main": continue
                if f.return_type: program += f.return_type
                else: program += "static void"
                program += " " + f.identifier + "();\n"

            for i, f in enumerate(functions):
                if f.return_type: program += f.return_type
                else: program += "static void"
                program += " " + f.identifier + "() {"
                if len(f.statements) > 0:
                    program += "\n"
                    for s in f.statements:
                        program += "    " + s + "\n"
                else: program += "\n"

                none_if_for = randint(0, 4)
                is_if = none_if_for == 1
                is_for = none_if_for == 2
                if is_if:
                    program += "    if(0) {\n"
                elif is_for:
                    program += "    for(;0;) {\n"

                # Generate variable declaration
                variable_id = 0
                while randint(1, 100) <= 40:
                    program += f"    int var{variable_id} = 0;\n"
                    if variable_id > 0:
                        program += f"    var{variable_id} = var{randint(0, variable_id)};\n"
                    variable_id += 1

                # Generate function call
                viable_idx = list(function_ids)
                if i < len(viable_idx) and i in viable_idx:
                    viable_idx.remove(i)

                while len(viable_idx) > 0 and randint(1, 3) == 1:
                    idx = choice(viable_idx)
                    viable_idx.remove(idx)
                    program += f"    f{idx}();\n"

                if is_if or is_for:
                    program += "    }\n"

                while randint(1, 100) <= 40:
                    program += f"    int var{variable_id} = 0;\n"
                    variable_id += 1

                # Generate function call
                if i < len(viable_idx) and i in viable_idx:
                    viable_idx.remove(i)

                while len(viable_idx) > 0 and randint(1, 3) == 1:
                    idx = choice(viable_idx)
                    viable_idx.remove(idx)
                    program += f"    f{idx}();\n"

                program += "}\n"

            f = os.path.join(dir_name, f"{file_name}.c")
            with open(f, "w") as c_file:
                c_file.write(program)
