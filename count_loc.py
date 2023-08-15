import os
import re

def count_lines(filename, ext):
    """Count the lines of code while ignoring comments."""
    with open(filename, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()

    if ext in ['.c', '.cpp', '.h', '.hpp']:
        # C and C++ single line comments start with //
        # C and C++ multi-line comments are between /* and */
        content = re.sub(r'//.*', '', content)  # Remove single line comments
        content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)  # Remove multi-line comments
    elif ext == '.py':
        content = re.sub(r'#.*', '', content)  # Remove Python single line comments
    elif ext == '.lua':
        # Lua single line comments start with --
        # Lua multi-line comments are between --[[ and ]]
        content = re.sub(r'--.*', '', content)  # Remove single line comments
        content = re.sub(r'--\[\[.*?\]\]', '', content, flags=re.DOTALL)  # Remove multi-line comments

    return len([line for line in content.split('\n') if line.strip()])  # Count non-empty lines

def main(directory="."):
    loc_by_language = {
        'C/C++': 0,
        'Python': 0,
        'Lua': 0
    }

    # Walk through the directory
    for dirpath, dirnames, filenames in os.walk(directory):
        for filename in filenames:
            ext = os.path.splitext(filename)[1]
            if ext in ['.c', '.cpp', '.h', '.hpp']:
                loc_by_language['C/C++'] += count_lines(os.path.join(dirpath, filename), ext)
            elif ext == '.py':
                loc_by_language['Python'] += count_lines(os.path.join(dirpath, filename), ext)
            elif ext == '.lua':
                loc_by_language['Lua'] += count_lines(os.path.join(dirpath, filename), ext)

    # Display results
    for lang, loc in loc_by_language.items():
        print(f"{lang}: {loc} lines of code")

if __name__ == "__main__":
    # By default, start with the current directory.
    main(".")




