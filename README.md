<p align="center"><img src="https://i.imgur.com/pwlmRZx.png" width="150" height="150"></p>
<h2 align="center"><b>NODA - Your IDA buddy.</b></h2>
NODA is an IDA Plugin which enhances your experience while reverse engineering with IDA-Pro.

## Installation

Simply drag the plugin into your IDA Plugin directory.

### Building

# Build dependencies

* A Windows machine with Visual Studio 2019 (Community Edition is fine too)
* Qt 5.6.3 with `msvc2015_64` (Ensure that the `QTDIR` or `QT_DIR` environment variable are set). It can be found [here](https://download.qt.io/new_archive/qt/5.6/5.6.3/)

# Setup the project

```
git clone https://github.com/NomadGroup/NODA.git
cd NODA
git submodule update --init
```
#### Building the project

Execute pmake.cmd in order to generate the project solution. 

Afterwards you can open `build/NODA.sln`. The recommended build configuration is `Release` for all purposes.