import os
import re

def find_stl_functions(filename):
    try:
        with open(filename, 'r', encoding='utf-8') as f:
            contents = f.readlines()
    except UnicodeDecodeError:
        print(f"{filename}: Error reading file (it might not be in UTF-8 format)")
        return  # exit the function early

    # A regex pattern to detect std:: followed by any word (considered function or type)
    pattern = re.compile(r'\bstd::(\w+)\b')

    for idx, line in enumerate(contents, 1):
        matches = pattern.findall(line)
        for match in matches:
            print(f"{filename}:{idx}: stl sym: std::{match}")

if __name__ == "__main__":
    # Iterating through all files in current directory
    for root, dirs, files in os.walk('.'):
        for file in files:
            if file.endswith(('.h', '.cc')) and not file.endswith('_test.cc'):
                find_stl_functions(os.path.join(root, file))
