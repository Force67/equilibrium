# \Equilibrium
Equilibrium is a framework for assembly related things, that aims to deliver a stable and enjoyable experience for reverse engineering/obfuscating code.

# Architecture
* `base/`: Core library used by all projects.
* `build/`: Lua buildscripts and build configuration.
* `external/`: External dependencies.
* `components/`: Pieces of code shared between multiple different projects.
* `nemisis/`: The NEMISIS obfuscator.
* `retk/`: Reverse Engineering ToolKit tree.
* `tools/`: Various tools to quickly do stuff.

# Building

First clone the repo, like below:
```
git clone https://github.com/Force67/equilibrium.git
cd equilibrium
git submodule update --init
```
Afterwards, execute pmake.cmd or pmake.sh depending on your operating system in order to generate the files required for building. You will find them in the `./out/` directory.