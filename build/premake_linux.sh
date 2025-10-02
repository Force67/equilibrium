
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
TOTAL_START=$(date +%s%N)

# Define valid build modes
declare -a BUILD_MODES=("debug" "release" "shipping" "ci_release")

if [[ $# -eq 0 ]]; then
    echo 'No target configuration provided (e.g., debug, release, etc.).'
    echo "Valid build modes: ${BUILD_MODES[*]}"
    exit 1
fi

BUILD_MODE=$1
VSCODE_TRIGGER=false

# Check for --vscode flag
if [[ "$2" == "--vscode" ]]; then
    VSCODE_TRIGGER=true
fi

# Check if build mode is valid
if [[ ! " ${BUILD_MODES[@]} " =~ " ${BUILD_MODE} " ]]; then
    echo "Invalid build mode: $BUILD_MODE"
    echo "Valid build modes: ${BUILD_MODES[*]}"
    exit 1
fi

# Set environment variable for CI builds
if [[ "$BUILD_MODE" == "ci_release" ]]; then
    export BLU_IS_CI_BUILD=1
    BUILD_MODE="release"  # Use release configuration for CI builds
fi

if [[ ! -f "$PREMAKE_BIN" ]]; then
    echo "Premake5 binary not found at $PREMAKE_BIN"
    exit 1
fi

run_premake() {
    local args=("$@")
    local label="${args[*]}"
    local start_ts=$(date +%s%N)

    "$PREMAKE_BIN" --file="$PREMAKE_FILE" "${args[@]}"
    local status=$?

    local end_ts=$(date +%s%N)
    local elapsed_ns=$((end_ts - start_ts))
    local elapsed_ms=$((elapsed_ns / 1000000))
    local seconds=$((elapsed_ms / 1000))
    local millis=$((elapsed_ms % 1000))

    if [[ $status -ne 0 ]]; then
        printf 'premake %s failed after %d.%03ds\n' "$label" "$seconds" "$millis"
        exit 1
    fi

    printf 'premake %s finished in %d.%03ds\n' "$label" "$seconds" "$millis"
}

case "$TERM_PROGRAM" in
    vscode)
        run_premake vscode --vscode-config=$BUILD_MODE --vscode-fakeaction=gmake2
        ;;
    clion)
        echo 'Running in CLion'
        ;;
esac

# Trigger vscode run from terminal if --vscode is passed
if [[ "$VSCODE_TRIGGER" = true ]]; then
    run_premake vscode --vscode-config=$BUILD_MODE --vscode-fakeaction=gmake2
fi


# ignore in ci builds
# ignore in ci builds
# ignore in ci builds
if [[ -z "$BLU_IS_CI_BUILD" ]]; then
    run_premake \
        --export-compile-config=$BUILD_MODE \
        --zed-configs=debug,release \
        --zed-fakeaction=gmake2 \
        gmake2
else
    run_premake \
        --skip-compile-commands \
        --zed-configs=debug,release \
        --zed-fakeaction=gmake2 \
        gmake2
fi

TOTAL_END=$(date +%s%N)
TOTAL_NS=$((TOTAL_END - TOTAL_START))
TOTAL_MS=$((TOTAL_NS / 1000000))
TOTAL_SECONDS=$((TOTAL_MS / 1000))
TOTAL_REMAINDER_MS=$((TOTAL_MS % 1000))
printf 'total premake workflow finished in %d.%03ds\n' "$TOTAL_SECONDS" "$TOTAL_REMAINDER_MS"

exit 0
