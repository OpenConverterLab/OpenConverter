#!/bin/sh
# Wrapper launcher for OpenConverter
# Sets up library paths before launching the real executable

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Set DYLD_LIBRARY_PATH for Python framework
export DYLD_LIBRARY_PATH="$HOME/Library/Application Support/OpenConverter/Python.framework/lib${DYLD_LIBRARY_PATH:+:$DYLD_LIBRARY_PATH}"

# Execute the real binary
exec "$SCRIPT_DIR/OpenConverter.real" "$@"

