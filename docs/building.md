# Prerequisites
Ensure that clang format is installed on your machine and available via the global path.

A build machine with >8GB RAM is *strongly* recommended.

# First steps

First clone the repo from the git repository, like below:
```
git clone https://github.com/Force67/equilibrium.git
cd equilibrium
git submodule update --init
```
Afterwards, execute `build/premake_windows.cmd` or `build\premake_linux.sh` depending on your operating system in order to generate the files required for building. You will find them in the `/out/` directory.

## Building on windows
Execute `build/premake_windows.cmd` and afterwards you will find the solution generated in the `/out/` directory.
Make sure that you have ASAN installed in the Visual Studio installer.

## Building on linux
Execute `build\premake_linux.sh` with the target config as the first parameter:
```
$ ./premake_linux.sh debug
```

> **⚠️ Warning**
> Always invoke the build steps using the script, because the script ensures proper environment variables and IDE support.

This step generates a matching proper compile_commands.json for clangd, as well as makefiles which can be found in the `/out/gmake2` directory.
Target config values are defined in the root equilibrium directory premake5.lua file or you may pass `all` in order to generate different compile commands
for each configuration. Just be aware that these don't update the main compile_commands.json in the root directory in this case.
Afterwards, just ```make``` in the gmake2 directory mentioned before, and you should be able to compile everything as intended.

## IDE support on linux
### VSCode
`premake_linux.sh` will detect the presence of VSCode, and generate the proper tasks.json

# I managed to build, what's next?
Read the [developer guide](developer_guide.md) on how to get started writing code.