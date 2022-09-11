#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [[ $# -eq 0 ]] ; then
    echo 'No target configuration provided (can be debug, release, etc.)'
    exit 0
fi

if [[ "$TERM_PROGRAM" == "vscode" ]]; then
  "$DIR/bin/linux/premake5" --file="$DIR/../premake5.lua" vscode --vscode-config=$1 --vscode-fakeaction=gmake2
fi

if [[ "$TERM_PROGRAM" == "clion" ]]; then
  echo 'hello from clion'
fi

"$DIR/bin/linux/premake5" --file="$DIR/../premake5.lua" export-compile-commands --export-compile-config=$1
"$DIR/bin/linux/premake5" --file="$DIR/../premake5.lua" gmake2