# Base

Base covers the basic needs of a cxx application in a portable, uniform way.

- Portable, and covers all basic needs of a modern cxx application.
- Maximum test coverage.
- Sensible modern cxx, no STL where possible.
- Speed by default.
- Customizable: disable the parts you do not need.

Every class should have a unit test file. Files that do are safe to use in production.

## No STL, and as little CRT as possible

`find_runtime_deps.py` reports what is left of both and separates what could
be removed from what cannot.

### STL

- **Core language, not library.** `<new>` for placement new, and
  `<initializer_list>` for the brace-init constructors that the language
  resolves to `std::initializer_list` by fiat. Both headers are freestanding.
  `std::nothrow_t` is in the same bucket: the ABI fixes the signature of the
  `operator new` replacements.
- **Deliberate interop.** The `std::formatter` specializations for base's
  string types, so a consumer that formats one through `std::format` gets its
  text rather than C++23 range formatting spelling it out character by
  character. `BASE_NO_STD_FORMAT` removes them and the `<format>` include.
- **Escape hatches, off by default.** `BASE_USE_STD_ATOMIC` and
  `BASE_USE_STD_MUTEX` route `base::Atomic` and `base::Mutex` back through
  `<atomic>` and `<mutex>`. Nothing sets either; they are there for a compiler
  neither native backend covers.

### C runtime

The line is drawn at the runtime, not the operating system. `write`, `open`,
`mmap` and `pthread_*` are how a process talks to the kernel and base uses
them freely. Buffered stdio, the printf family, `malloc` and the locale
machinery are a library that happens to ship alongside, and base does not.

| Instead of | Use |
| --- | --- |
| `<string.h>` mem functions | `base/memory/mem_ops.h` |
| `strlen`, `strchr`, `strrchr` | `base/strings/char_algorithms.h` |
| `strcmp`, `strncmp` | `base/strings/string_compare.h` |
| `snprintf` | `base::FormatTo`, `base/strings/format.h` |
| `strtoll`, `strtod` | `base/strings/number_parse.h` |
| number to text | `base::ToString`, `base/strings/to_string.h` |
| `fprintf(stderr, ...)`, `abort` | `base/standard_streams.h` |
| `getenv`, `setenv` | `base/environment_variables.h` |
| `strerror` | `base::ErrnoName`, `base/system_error.h` |
| `fopen` + `fgets` over `/proc` | `base::ReadSmallFile` |

Number conversion is exact in both directions rather than approximate, and
both are checked against the C functions over millions of values rather than
against a reading of the standard. `BASE_FLOAT_FUZZ_ITERATIONS` and
`BASE_NUMBER_FUZZ_ITERATIONS` raise the sweep counts for a deep run.

What stays: `allocator/default_crt_alloc.h`, which is the CRT allocator
router and exists to call `malloc`; the `free()` in `debugging.cc` for the
buffers `__cxa_demangle` and `backtrace_symbols` hand back, which their
contracts require; `setenv`/`unsetenv`, which own the storage they allocate;
and the Windows entry-point shim, whose whole job is CRT integration.

The C library under its C++ spelling (`<cstring>`, `<cstdio>`) is worse
than either: it only promises the `std::` overloads, so the bare `memcpy` a
call site writes compiles by a standard library's courtesy rather than by
guarantee. Those headers are gone from base entirely.
