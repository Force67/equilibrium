# Prerequisites
Ensure that clang format is installed on your machine and available via the global path.

A build machine with >8GB RAM is *strongely* recommended.

# First steps

First clone the repo from the git repository, like below:
```
git clone https://github.com/Force67/equilibrium.git
cd equilibrium
git submodule update --init
```
Afterwards, execute pmake.cmd or pmake.sh depending on your operating system in order to generate the files required for building. You will find them in the `/out/` directory.

## Building for windows
Make sure that you have ASAN installed in the Visual Studio installer.
## Building for linux
On Linux gmake2 is used, so you will have to execute `make` in the `/out/gmake2` directory.

# I managed to build, what's next?
Read the [developer guide](developer_guide.md) on how to get started writing code.