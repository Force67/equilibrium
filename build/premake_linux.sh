resolve_symlink() {
  local script="$1"
  local resolved_path="$script"

  while [ -L "$resolved_path" ]; do
    resolved_path=$(readlink -f "$resolved_path")
  done

  echo "$(dirname "$resolved_path")"
}

if [[ -z "$EQUILIBRIUM_NIX" ]]; then
    echo 'warning: You have not invoked the build process from within a nix environment. This can cause compile issues.'
fi

# Resolve the directory of the current script
DIR=$(resolve_symlink "${BASH_SOURCE[0]}")

PREMAKE_BIN="$DIR/bin/linux/premake5"
PREMAKE_FILE="$DIR/../premake5.lua"

if [[ $# -eq 0 ]]; then
    echo 'No target configuration provided (e.g., debug, release, etc.).'
    exit 1
fi

BUILD_MODE=$1

if [[ "$BUILD_MODE" != "debug" && "$BUILD_MODE" != "release" && "$BUILD_MODE" != "shipping" ]]; then
    echo "Invalid build mode: $BUILD_MODE. Allowed values are: debug, release."
    exit 1
fi

if [[ ! -f "$PREMAKE_BIN" ]]; then
    echo "Premake5 binary not found at $PREMAKE_BIN"
    exit 1
fi

run_premake() {
    "$PREMAKE_BIN" --file="$PREMAKE_FILE" "$@"
    if [[ $? -ne 0 ]]; then
        echo "Premake5 command failed: $@"
        exit 1
    fi
}

case "$TERM_PROGRAM" in
    vscode)
        run_premake vscode --vscode-config=$BUILD_MODE --vscode-fakeaction=gmake2
        ;;
    clion)
        echo 'Running in CLion'
        ;;
esac

run_premake export-compile-commands --export-compile-config=$BUILD_MODE
run_premake gmake2

exit 0
