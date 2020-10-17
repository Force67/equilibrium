
<p align="center"><img src="https://i.imgur.com/pwlmRZx.png" width="150" height="150"></p>

# Noda
> By [Nomad Group](https://www.nomad-group.net/) &middot; [Demo](https://i.imgur.com/pwlmRZx.png)
> 
**Noda** is a set of tools designed to enhance your reverse engineering experience. It seamlessly works with IDA Pro and X64Debug.

## Table of contents
- [Features](#features)
- [Setup](#setup)
- [Building](#building)

# Features

- **Noda-Sync**: Synchronize your reverse engineering among IDA-Pro and X64Dbg instances in real time.
- **Snapshots**: Noda allows you to save and load different snapshots.
- **Inbuilt Tools**: Noda enhances the reverse engineering experience by adding tools such as PetternTool.

# Setup
The noda server requires no configuration. TODO

# Building

First, [install Qt 5.6.3](https://download.qt.io/new_archive/qt/5.6/5.6.3/), which is required for the IDA plugin. Then clone the repo, like below:
```
git clone https://github.com/NomadGroup/NODA.git
cd NODA
git submodule update --init
```
#### Building the project

Execute pmake.cmd in order to generate the project solution. 

Afterwards you can open `build/NODA.sln`. The recommended build configuration is `Release` for all purposes.