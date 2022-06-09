#Generate latest compile-commands db
./bin/linux/premake5 --file=../premake5.lua export-compile-commands
#Generate makefile
./bin/linux/premake5 --file=../premake5.lua gmake2