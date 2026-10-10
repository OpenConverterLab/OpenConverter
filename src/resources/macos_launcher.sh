#!/bin/sh
# Copyright 2026 Jack Lau
# Email: jacklau1222gm@gmail.com
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Wrapper launcher for OpenConverter
# Sets up library paths before launching the real executable

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Set DYLD_LIBRARY_PATH for Python framework
export DYLD_LIBRARY_PATH="$HOME/Library/Application Support/OpenConverter/Python.framework/lib${DYLD_LIBRARY_PATH:+:$DYLD_LIBRARY_PATH}"

# Execute the real binary
exec "$SCRIPT_DIR/OpenConverter.real" "$@"
