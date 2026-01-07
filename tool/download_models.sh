#!/usr/bin/env bash
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

# Script to download AI model weights for OpenConverter
# This script is called during the build process to download required model files

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
WEIGHTS_DIR="$PROJECT_ROOT/src/modules/weights"

# Create weights directory if it doesn't exist
mkdir -p "$WEIGHTS_DIR"

echo "========================================="
echo "Downloading AI Model Weights"
echo "========================================="

# Define models as separate arrays (compatible with older bash versions)
MODEL_NAME="realesr-animevideov3.pth"
MODEL_URL="https://github.com/xinntao/Real-ESRGAN/releases/download/v0.2.5.0/realesr-animevideov3.pth"
MODEL_PATH="$WEIGHTS_DIR/$MODEL_NAME"

if [ -f "$MODEL_PATH" ]; then
    echo "✓ $MODEL_NAME already exists, skipping download"
else
    echo "⬇ Downloading $MODEL_NAME..."
    if command -v curl &> /dev/null; then
        curl -L -o "$MODEL_PATH" "$MODEL_URL"
    elif command -v wget &> /dev/null; then
        wget -O "$MODEL_PATH" "$MODEL_URL"
    else
        echo "Error: Neither curl nor wget is available. Please install one of them."
        exit 1
    fi

    if [ -f "$MODEL_PATH" ]; then
        echo "✓ Successfully downloaded $MODEL_NAME"
    else
        echo "✗ Failed to download $MODEL_NAME"
        exit 1
    fi
fi

echo "========================================="
echo "All model weights are ready!"
echo "========================================="
