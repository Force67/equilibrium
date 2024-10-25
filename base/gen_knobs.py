#!/usr/bin/env python3

import os
import re
import argparse
from typing import List, Tuple

def find_knobs(directory: str) -> List[Tuple[str, str, str]]:
    knobs = []
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith('.cc'):
                with open(os.path.join(root, file), 'r') as f:
                    content = f.read()
                    matches = re.findall(r'base::Knob<(\w+)>\s+(\w+)(?:\s*=\s*)?([^;]*);', content)
                    for match in matches:
                        knob_type, knob_name, initializer = match
                        knobs.append((knob_type, knob_name, initializer.strip()))
    return knobs

def generate_init_header(knobs: List[Tuple[str, str, str]], output_file: str):
    with open(output_file, 'w') as f:
        f.write("// Copyright (C) 2024 Vincent Hengel.\n")
        f.write("// For licensing information see LICENSE at the root of this distribution.\n")
        f.write("//\n")
        f.write("// Auto-generated file. Do not edit manually.\n\n")
        f.write("#pragma once\n\n")
        f.write("#include <base/knob.h>\n\n")
        f.write("namespace feature_flags {\n\n")

        # Declare extern Knobs
        for knob_type, knob_name, _ in knobs:
            f.write(f"extern base::Knob<{knob_type}> {knob_name};\n")

        f.write("\ninline void InitializeAllKnobs() {\n")
        for knob_type, knob_name, initializer in knobs:
            #if initializer:
            #    f.write(f"    {knob_name}.Construct({initializer});\n")
            #else:
            f.write(f"    {knob_name}.Construct();\n")
        f.write("}\n\n")

        f.write("inline void DestructAllKnobs() {\n")
        for _, knob_name, _ in knobs:
            f.write(f"    {knob_name}.Destruct();\n")
        f.write("}\n\n")

        f.write("struct KnobEntry {\n")
        f.write("    const char* name;\n")
        f.write("    base::BasicKnob* knob_obj;\n")
        f.write("};\n\n")

        f.write(f"constexpr int kKnobCount = {len(knobs)};\n\n")

        f.write("inline void InitializeAllKnobsAndRegister(KnobEntry (&knob_list)[kKnobCount]) {\n")
        f.write("    InitializeAllKnobs();\n")
        for i, (_, knob_name, _) in enumerate(knobs):
            snake_case = re.sub(r'(?<!^)(?=[A-Z])', '_', knob_name).lower()
            f.write(f'    knob_list[{i}] = KnobEntry{{"{snake_case}", &{knob_name}}};\n')
        f.write("}\n\n")

        f.write("}  // namespace feature_flags\n")

def main():
    parser = argparse.ArgumentParser(description="Generate Knob initialization header")
    parser.add_argument("source_dir", help="Source directory to search for .cc files")
    parser.add_argument("output_file", help="Output header file")
    args = parser.parse_args()

    knobs = find_knobs(args.source_dir)
    generate_init_header(knobs, args.output_file)
    print(f"Generated {args.output_file} with {len(knobs)} knobs.")

if __name__ == "__main__":
    main()
