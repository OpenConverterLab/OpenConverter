#!/bin/bash
# Copyright 2025 Jack Lau
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

# Exit on error
set -e

# Get the root directory of the project
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
THIRD_PARTY_DIR="${ROOT_DIR}/third_party"

# Create third_party directory if it doesn't exist
mkdir -p "${THIRD_PARTY_DIR}"

# Deinitialize all submodules first
echo "Deinitializing existing submodules..."
git submodule deinit -f --all || true

# Remove .gitmodules if it exists
if [ -f "${ROOT_DIR}/.gitmodules" ]; then
    echo "Removing existing .gitmodules..."
    rm "${ROOT_DIR}/.gitmodules"
fi

# Create fresh .gitmodules
echo "Creating new .gitmodules file..."
touch "${ROOT_DIR}/.gitmodules"
git add "${ROOT_DIR}/.gitmodules"

# Initialize git submodules
echo "Initializing git submodules..."
# git submodule init

# Add or update googletest submodule
if [ ! -d "${THIRD_PARTY_DIR}/googletest" ]; then
    echo "Adding googletest submodule..."
    git submodule add https://github.com/google/googletest.git "${THIRD_PARTY_DIR}/googletest"
    # git submodule update --init --recursive "${THIRD_PARTY_DIR}/googletest"
    (cd "${THIRD_PARTY_DIR}/googletest" && git checkout release-1.12.1)
fi

echo "Test dependencies have been set up successfully!"
