import os
import subprocess
import sys
from pathlib import Path

def format_files(root_dirs):
    # Get the directory where the script is located
    script_dir = os.path.dirname(os.path.abspath(__file__))

    # Path to .clang-format file
    clang_format_path = os.path.join(script_dir, '.clang-format')

    # Check if .clang-format exists
    if not os.path.exists(clang_format_path):
        print("Error: .clang-format file not found in script directory")
        sys.exit(1)

    # Extensions to format
    extensions = ('.cc', '.h')

    # Find and format files
    for root_dir in root_dirs:
        if not os.path.exists(root_dir):
            print(f"Warning: Directory {root_dir} does not exist")
            continue

        for root, _, files in os.walk(root_dir):
            for file in files:
                if file.endswith(extensions):
                    file_path = os.path.join(root, file)
                    print(f"Formatting: {file_path}")
                    try:
                        subprocess.run(['clang-format', '-i', '-style=file', file_path], check=True)
                    except subprocess.CalledProcessError as e:
                        print(f"Error formatting {file_path}: {e}")
                    except FileNotFoundError:
                        print("Error: clang-format not found. Please install clang-format.")
                        sys.exit(1)

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python format_code.py <directory1> [directory2 ...]")
        sys.exit(1)

    format_files(sys.argv[1:])
