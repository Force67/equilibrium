#Generate latest compile-commands.json
./bin/linux/premake5 --file=../premake5.lua export-compile-commands --export-compile-config=$1
#Generate makefiles
./bin/linux/premake5 --file=../premake5.lua gmake2