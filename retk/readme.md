# `retk/`
# ![Logo](https://i.imgur.com/W7tqzZg.png) **R**everse **E**ngineering **T**ool**K**it

**RETK** is tool for decompiling applications back to human readable code. 
It also comes with integrations (plugins) for IDA Pro and X64Dbg.

## Table of contents
- [Features](#features)
- [Installation](#installation)
- [Building](#building)
- [Plugins](#plugins)
- [Credits](#credits)

# Features

- **Realtime sync**: Synchronize your reverse engineering between instances of RETK/IDA-Pro/X64Dbg in real time.
- **Inbuilt Tools**: RETK enhances the reverse engineering experience by adding tools such as PatternTool.

# Building

For building the IDA plugin, you'll need a copy of the IDA plugin sdk and Qt 5.6.3, which you can find [here](https://download.qt.io/new_archive/qt/5.6/5.6.3/)

# Installation

Copy `IDA_Retk76` into the `/IDA PRO/plugins` directory. <br/>
Copy `x64dbg_RETK` into the `X64Dbg/release/x64 (or x32)/plugins` directory.

# Credits
* [Freeeaky](https://github.com/Freeeaky) for his original [sync plugin](https://github.com/Nomad-Group/IDASync), which was one of the inspirations for creating RETK.
* [Fontawesome](https://fontawesome.com) for icons used in the IDA plugin.
