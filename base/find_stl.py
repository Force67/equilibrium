import os
import re

def find_stl_functions(filename, stats):
    try:
        with open(filename, 'r', encoding='utf-8') as f:
            contents = f.readlines()
    except UnicodeDecodeError:
        print(f"{filename}: Error reading file (it might not be in UTF-8 format)")
        return  # exit the function early

    # A regex pattern to detect std:: followed by any word (considered function or type)
    pattern = re.compile(r'\bstd::(\w+)\b')
    inside_comment_block = False

    for idx, line in enumerate(contents, 1):
        # Update the total lines count
        stats['total_lines'] += 1

        # Check if line is inside a block comment
        if inside_comment_block:
            if '*/' in line:
                inside_comment_block = False
            continue

        # Check if the line starts a block comment
        if '/*' in line:
            inside_comment_block = True
            continue

        # Check if the line is a single-line comment
        if line.strip().startswith('//'):
            continue

        matches = pattern.findall(line)
        if matches:
            # Update the count of lines with STL functions
            stats['stl_lines'] += 1
            for match in matches:
                print(f"{filename}:{idx}: stl sym: std::{match}")

def calculate_percentage(stats):
    if stats['total_lines'] == 0:
        return 0
    percentage = (stats['stl_lines'] / stats['total_lines']) * 100
    rounded_percentage = round(percentage, 2)
    return rounded_percentage

if __name__ == "__main__":
    stats = {
        'total_lines': 0,
        'stl_lines': 0
    }

    # Iterating through all files in current directory
    for root, dirs, files in os.walk('.'):
        for file in files:
            if file.endswith(('.h', '.cc')) and not file.endswith('_test.cc'):
                find_stl_functions(os.path.join(root, file), stats)
                
    percentage = calculate_percentage(stats)
    print(f"{percentage}% of code contains STL funcitons")
