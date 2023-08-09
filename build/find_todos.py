import os
import re

def find_todos(filename):
    try:
        with open(filename, 'r', encoding='utf-8') as f:
            contents = f.readlines()
    except UnicodeDecodeError:
        print(f"{filename}: Error reading file (it might not be in UTF-8 format)")
        return  # exit the function early

    # A regex pattern to detect TODO
    todo_pattern = re.compile(r'TODO', re.IGNORECASE)

    for idx, line in enumerate(contents, 1):
        if todo_pattern.search(line):
            print(f"{filename}:{idx}: TODO detected: {line.strip()}")

if __name__ == "__main__":
    # Iterating through all files in current directory
    for root, dirs, files in os.walk('.'):
        for file in files:
            if file.endswith(('.h', '.cc')) and not file.endswith('_test.cc'):
                find_todos(os.path.join(root, file))