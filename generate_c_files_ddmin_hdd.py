from dataclasses import dataclass
from datetime import datetime
from random import randint, sample
import os


@dataclass
class Function:
    identifier: str = None
    return_type: str = None
    statements = []


NUM_FUNCTIONS = [12, 16, 20, 24]
NUM_RELEVANTS = [2, 6, 10]


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
            # Add random variables
            # max_id = max(num_functions >> 1, 4)
            # min_id = num_functions >> 2
            # var_ids = [i for i in range(randint(min_id, max_id))]
            # for i in var_ids:
            #     program += f"static int var{i} = 0"
            #     if i > 0 and randbool():
            #         # Add operator for depth in AST
            #         program += f" + 0"

            #     program += ";\n"

            for i, f in enumerate(functions):
                if i >= len(function_ids): i = None
                if f.return_type: program += f.return_type
                else: program += "static void"
                program += " " + f.identifier + "() {"
                if len(f.statements) > 0:
                    program += "\n"
                    for s in f.statements:
                        program += "    " + s + "\n"
                else: program += "\n"

                # Add random statements
                # Add brackets so the AST is deeper
                # add_brackets = randbool()
                # if add_brackets: program += "    {\n"
                # for _ in range(randint(min_id, max_id - 2)):
                #     # Add dependency using variables
                #     if randbool():
                #         program += f"    var{randint(0, len(var_ids) - 1)} = 0"
                #         # More depth
                #         while randbool():
                #             if randbool():
                #                program += f" + var{randint(0, len(var_ids) - 1)}"
                #             else:
                #                 program += f" + 0"
                #         program += ";\n"
                #         if add_brackets and randbool():
                #             program += "    }\n"
                #             add_brackets = False

                #     # Add dependencies using function calls
                #     if i:
                #         func_id = randint(0, i)
                #         tries = 0
                #         while func_id in relevant_functions or func_id == i:
                #             func_id = randint(0, i)
                #             tries += 1
                #             if tries > 50: break

                #         program += f"    f{func_id}();\n"

                # if add_brackets: program += "    }\n"


                program += "}\n"

            f = os.path.join(dir_name, f"{file_name}.c")
            with open(f, "w") as c_file:
                c_file.write(program)
