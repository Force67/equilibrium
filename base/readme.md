# Base

Base covers the basic needs of a cxx application in a portable, uniform way.

- Portable, and covers all basic needs of a modern cxx application.
- Maximum test coverage.
- Sensible modern cxx, no STL where possible.
- Speed by default.
- Customizable: disable the parts you do not need.

Every class should have a unit test file. Files that do are safe to use in production.

## No STL

base does not use the STL. `find_stl.py` reports what is left and separates
what could be removed from what cannot:

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

The C library is not the STL, but `<cstring>` and friends are: they only
promise the `std::` overloads, so the bare `memcpy` base spells everywhere
compiles by a standard library's courtesy rather than by guarantee. Include
`<string.h>`, `<stdio.h>` and the rest under their C names.
