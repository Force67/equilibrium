
<p align="center"><img src="https://i.imgur.com/lOLpWuF.png" width="150" height="150"></p>

# **R**everse **E**ngineering **T**ool**k**it
> By [Force67](https://github.com/Force67) &middot; [Demo](https://i.imgur.com/pwlmRZx.png)
> 
**RETK** is a set of tools designed to enhance your reverse engineering experience. It seamlessly works with IDA Pro and X64Debug.

## Table of contents
- [Features](#features)
- [Setup](#setup)
- [Building](#building)

# Features

- **RESync**: Synchronize your reverse engineering among IDA-Pro and X64Dbg instances in real time.
- **Snapshots**: Noda allows you to save and load different snapshots.
- **Inbuilt Tools**: Noda enhances the reverse engineering experience by adding tools such as PatternTool.

# Setup
Copy IDA_Retk76.dll into your IDA plugin directory.

# Building
First, [install Qt 5.6.3](https://download.qt.io/new_archive/qt/5.6/5.6.3/), which is required for the IDA plugin. Then clone the repo, like below:
```
git clone https://github.com/Force67/RETK.git
cd RETK
git submodule update --init
```
#### Building the project
Execute pmake.cmd in order to generate the project solution. 

Afterwards you can open `build/RETK.sln`. The recommended build configuration is `Release` for all purposes.

# Credits
* [Freeeaky](https://github.com/Freeeaky) for his original [sync plugin](https://github.com/Nomad-Group/IDASync), which inspired me to create RETK.
* [Fontawesome](https://fontawesome.com) for icons used in the IDA plugin!
