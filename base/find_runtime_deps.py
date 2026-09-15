# Reports base's remaining STL and C runtime dependencies, split by whether
# each is something we can actually remove. Run from base/:
#
#     python3 find_runtime_deps.py [--all]
#
# base's policy is "no STL where possible". Three things are not possible, and
# reporting them as violations every run is what made this script easy to
# ignore:
#
#   - <new> and placement new, and <initializer_list> with the
#     std::initializer_list constructors it exists for. Both are core language,
#     not library: brace-init picks std::initializer_list by fiat, and there is
#     no conforming way to declare placement new yourself.
#   - std::nothrow_t in the operator new replacements, whose signatures the ABI
#     fixes.
#   - The std::formatter specializations for base's string types, which exist
#     so a consumer that formats one through std::format gets its text instead
#     of C++23 range formatting spelling it out character by character. Opt out
#     with BASE_NO_STD_FORMAT.
#
# Anything else is a finding. The BASE_USE_STD_ATOMIC and BASE_USE_STD_MUTEX
# branches count: no configuration enables them, but they are still STL that
# base would compile if one did.
#
# The C runtime is tracked on the same terms. base spells its memory and string
# work through base/memory/mem_ops.h and base/strings/, formats and parses
# numbers itself, and writes diagnostics straight to the descriptors, so a
# <stdio.h> or <stdlib.h> turning up again is a regression worth seeing. Not
# everything can go: the CRT allocator router exists to call malloc, the
# demangler hands back memory only free() can release, and the thin POSIX and
# Win32 wrappers are the operating system interface rather than the runtime.

import os
import re
import sys

SYMBOL = re.compile(r'\bstd::(\w+)\b')
# The character class has to admit the '.' and digits in <stdio.h> and
# <inttypes.h>; without them this silently only ever saw STL headers.
INCLUDE = re.compile(r'^\s*#\s*include\s*<([a-z0-9_./]+)>')

# Core language and ABI, plus the deliberate std::format interop.
MANDATED_SYMBOLS = {
    'initializer_list',  # brace-init constructors
    'nothrow_t',         # operator new replacement signatures
    'formatter',         # opt-out interop, see BASE_NO_STD_FORMAT
    'basic_string_view', # only ever as the formatter's base
}
MANDATED_HEADERS = {
    'new',               # placement new
    'initializer_list',
    'format',            # opt-out interop
}

# Headers that are types and limits only, with no runtime behind them.
FREESTANDING_C_HEADERS = {'stdint.h', 'stddef.h', 'limits.h', 'float.h',
                          'stdbool.h', 'stdarg.h', 'iso646.h'}

# The C runtime proper: pulling one of these back in is the regression.
RUNTIME_C_HEADERS = {'stdio.h', 'stdlib.h', 'string.h', 'wchar.h', 'ctype.h',
                     'wctype.h', 'math.h', 'locale.h', 'setjmp.h', 'malloc.h',
                     'assert.h', 'inttypes.h'}

# Files whose whole purpose is to sit on the CRT.
CRT_BY_DESIGN = ('allocator/default_crt_alloc.h',
                 'bootstrap/executable_entry_point_win.in',
                 'debugging/debugging.cc')

SKIP_DIRS = {'external'}


def scan(path, stats, findings, mandated):
    try:
        with open(path, encoding='utf-8') as handle:
            lines = handle.readlines()
    except (UnicodeDecodeError, OSError) as error:
        print(f'{path}: could not read ({error})')
        return

    in_block_comment = False
    for number, line in enumerate(lines, 1):
        stats['total_lines'] += 1
        if in_block_comment:
            in_block_comment = '*/' not in line
            continue
        if '/*' in line and '*/' not in line:
            in_block_comment = True
            continue
        if line.lstrip().startswith(('//', '*')):
            continue

        header = INCLUDE.match(line)
        if header:
            name = header.group(1)
            if '.' in name:
                # Anything with an extension is a C or platform header, not an
                # STL one, including the .inl the Windows entry shim pulls in.
                if name in FREESTANDING_C_HEADERS:
                    continue
                if name not in RUNTIME_C_HEADERS:
                    continue  # An OS header, not the runtime.
                entry = f'{path}:{number}: include <{name}>  [CRT]'
                relative = path.replace(os.sep, '/')
                if relative.startswith('./'):
                    relative = relative[2:]
                if relative.startswith(CRT_BY_DESIGN):
                    mandated.append(entry)
                else:
                    findings.append(entry)
                    stats['stl_lines'] += 1
                continue
            bucket = mandated if name in MANDATED_HEADERS else findings
            bucket.append(f'{path}:{number}: include <{name}>')
            if name not in MANDATED_HEADERS:
                stats['stl_lines'] += 1
            continue

        names = SYMBOL.findall(line)
        if not names:
            continue
        if any(name not in MANDATED_SYMBOLS for name in names):
            stats['stl_lines'] += 1
        for name in names:
            bucket = mandated if name in MANDATED_SYMBOLS else findings
            bucket.append(f'{path}:{number}: std::{name}')


def main():
    show_mandated = '--all' in sys.argv
    stats = {'total_lines': 0, 'stl_lines': 0}
    findings, mandated = [], []

    for root, dirs, files in os.walk('.'):
        dirs[:] = [d for d in dirs if d not in SKIP_DIRS]
        for name in sorted(files):
            if not name.endswith(('.h', '.cc', '.in')):
                continue
            # Tests and the harnesses they run in are free to use whatever
            # they like; the point is what ships in the library.
            if name.endswith(('_test.cc', '_bench.cc')):
                continue
            if 'intrin_shim' in name or name == 'memory_unittests_main.cc':
                continue
            scan(os.path.join(root, name), stats, findings, mandated)

    if show_mandated:
        print(f'Mandated or deliberate ({len(mandated)}):')
        for entry in mandated:
            print(f'  {entry}')
        print()

    print(f'Removable STL and CRT use ({len(findings)}):')
    for entry in findings:
        print(f'  {entry}')
    if not findings:
        print('  none')

    share = stats['stl_lines'] / stats['total_lines'] * 100 if stats['total_lines'] else 0
    print(f'\n{share:.2f}% of {stats["total_lines"]} lines use removable STL or CRT')
    print(f'({len(mandated)} mandated or deliberate uses hidden; pass --all to list)')


if __name__ == '__main__':
    main()
