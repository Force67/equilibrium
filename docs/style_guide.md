# Style guide
For equilibrium we are following the [Google C++ style guide](https://google.github.io/styleguide/cppguide.html) with a few minor exceptions, mainly that we explicity use `#pragma once` instead of include guards. In addition *only* the following file extensions for C++ are permitted in new code:
* `.cc` for source files
* `.h` for header files
* `.in` for inline files

## Enforcing the style
The style should be automatically enforced on saving through clang format if you use Visual Studio. Additionally, all code is formatted in pre commit event.