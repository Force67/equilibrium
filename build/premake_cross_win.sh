DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

"$DIR/bin/linux/premake5" --file="$DIR/../premake5.lua" vs2022
