#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

#Generate latest compile-commands.json
"$DIR/bin/linux/premake5" --file="$DIR/../premake5.lua" export-compile-commands --export-compile-config=$1
#Generate makefiles
"$DIR/bin/linux/premake5" --file="$DIR/../premake5.lua" gmake2