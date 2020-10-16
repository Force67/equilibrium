
<p align="center"><img src="https://i.imgur.com/pwlmRZx.png" width="150" height="150"></p>
<h2 align="center"><b>Noda - a collaborative reverse-engineering project</b></h2>

### NODA comes with batteries included. Features include:
- IDA Sync: Synchronize your database.
- PDBTool: Generate fake PDB's directly from IDA.
- PatternTool: Generate your pattern cache directory from IDA.
- More planned :heart:

## Installation

Simply drag the plugin into your IDA Plugin directory.

# Building the project
First, [install Qt 5.6.3](https://download.qt.io/new_archive/qt/5.6/5.6.3/). Next, clone the repo, like below:
```
git clone https://github.com/NomadGroup/NODA.git
cd NODA
git submodule update --init
```
#### Building the project

Execute pmake.cmd in order to generate the project solution. 

Afterwards you can open `build/NODA.sln`. The recommended build configuration is `Release` for all purposes.