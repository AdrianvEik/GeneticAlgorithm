
import re
import os
import shutil

def parse_c_structures(file_path):
    """
    Parses a C header file to identify and extract structures, and returns a dictionary representation of the structures.

    Args:
    file_path (str): The path to the C header file.

    Returns:
    dict: A dictionary where keys are structure names and values are dictionaries of field names and their types.
    """
    struct_dict = {}
    parent_struct_dcit = {}
    struct_pattern = re.compile(r"^struct\s+(\w+)\s*\{$")
    field_pattern = re.compile(r"^\s*(int|double|char)(\*)?\s+(\w+);")
    child_struct_pattern = re.compile(r"^\s*struct\s+(\w+)\s+(\w+);$")
    end_struct_pattern = re.compile(r"^\};$")
    
    try:
        with open(file_path, 'r') as file:
            lines = file.readlines()

        current_struct = None
        for line in lines:
            if current_struct:
                # Check if we reached the end of the struct
                if end_struct_pattern.search(line):
                    current_struct = None
                else:
                    # Try to find a field definition
                    field_match = field_pattern.search(line)
                    child_struct_match = child_struct_pattern.search(line)
                    if field_match:
                        dtype, pointer, field_name = field_match.groups()
                        if pointer == "*":
                            struct_dict[struct_name][field_name] = f"ctypes.c_{dtype}_p"
                        else:
                            struct_dict[struct_name][field_name] = f"ctypes.c_{dtype}"
                    elif child_struct_match:
                        child_struct_type, child_struct_name = child_struct_match.groups()
                        struct_dict[struct_name][child_struct_name] = child_struct_type
            else:
                # Check for a new struct definition
                struct_match = struct_pattern.search(line)
                if struct_match:
                    struct_name = struct_match.group(1)
                    struct_dict[struct_name] = {}
                    current_struct = struct_name

    except FileNotFoundError:
        print("File not found. Please check the file path.")
    except Exception as e:
        print(f"An error occurred: {e}")
    
    return struct_dict


def generate_advanced_python_structs(struct_dict):
    """
    Generates Python ctypes structure definitions based on the dictionary representation of C structures.

    Args:
    struct_dict (dict): A dictionary where keys are structure names and values are dictionaries of field names and their types.

    Returns:
    str: A string containing the Python ctypes structure definitions.
    """
    ctype_pattern = re.compile(r"^ctypes\.c_")
    struct_definitions = ["import ctypes\n\n"]
    firstItem = True
    for struct_name, fields in struct_dict.items():
        struct_definition = f"class zz_{struct_name}__(ctypes.Structure):\n    _fields_ = ["
        firstItem = True
        for field_name, field_type in fields.items():
            if firstItem:
                firstItem = False
            else:
                struct_definition += ","
            if ctype_pattern.search(next(iter(fields.values()))):
                struct_definition += f"\n        (\"{field_name}\", {field_type})"
            else:
                struct_definition += f"\n        (\"{field_name}\", zz_{field_type}__)"
        struct_definition += "\n    ]\n\n"
        struct_definitions.append(struct_definition)

    for struct_name, fields in struct_dict.items():
        struct_definition = f"class {struct_name}:"
        for field_name, field_type in fields.items():
            if field_type == "ctypes.c_int":
                struct_definition += f"\n    {field_name} = 0"
            elif field_type == "ctypes.c_double":
                struct_definition += f"\n    {field_name} = 0.0"
            elif field_type == "ctypes.c_char_p":
                struct_definition += f"\n    {field_name}: str = \"\""
            else:
                struct_definition += f"\n    {field_name} = {field_type}()"
        struct_definition += "\n    def __init__(\n            self"
        for field_name, field_type in fields.items():
            if field_type == "ctypes.c_int":
                struct_definition += f",\n            {field_name}: int = 0"
            elif field_type == "ctypes.c_double":
                struct_definition += f",\n            {field_name}: float = 0.0"
            elif field_type == "ctypes.c_char_p":
                struct_definition += f",\n            {field_name}: str = \"\""
            else:
                struct_definition += f",\n            {field_name}: {field_type} = {field_type}()"
        struct_definition += "\n        ):\n"

        for field_name, field_type in fields.items():
            struct_definition += f"        self.{field_name} = {field_name}\n"
        struct_definition += f"    def cType(self):\n        return zz_{struct_name}__("
        firstItem = True
        for field_name, field_type in fields.items():
            if firstItem:
                firstItem = False
            else:
                struct_definition += ","
            if ctype_pattern.search(next(iter(fields.values()))):
                struct_definition += f"\n            {field_type}(self.{field_name})"
            else:
                struct_definition += f"\n            self.{field_name}.cType()"
        struct_definition += "\n        )\n\n"
        struct_definitions.append(struct_definition)
    return "\n".join(struct_definitions)

def generate_genetic_algorithm_object():
    
    
    pass

def generate_py_file(file_path, targetpath):
    """
    Generates a Python file containing ctypes structure definitions based on the C header file.

    Args:
    file_path (str): The path to the C header file.

    Returns:
    str: A string containing the Python ctypes structure definitions.
    """
    struct_dict = parse_c_structures(file_path)
    python_structs = generate_advanced_python_structs(struct_dict)
    
    with open(targetpath, 'w') as file:
        file.write(python_structs)
    
    print(f"Python ctypes structures generated and saved to {targetpath}")
    return None

if __name__ == "__main__":
    current_folder = os.getcwd()
    structheaderPath = f"{current_folder}\\Helper\\Struct.h"
    pythonstructpath = f"{current_folder}\\x64\\DLL Build\\Structs.py"
    generate_py_file(structheaderPath, pythonstructpath)
    shutil.copy(f"{current_folder}\\Genetic_Algorithm.py", f"{current_folder}\\x64\\DLL Build\\Genetic_Algorithm.py" )