#!/bin/bash

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
