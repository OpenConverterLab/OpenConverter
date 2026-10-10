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

# Simple DMG creation script without appdmg
# Usage: ./create_dmg_simple.sh [path_to_app]

set -e

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Get app path from parameter or use default
if [ -z "$1" ]; then
    # No parameter, use default path
    APP_PATH="$SCRIPT_DIR/../build/OpenConverter.app"
else
    # Parameter provided, resolve to absolute path
    INPUT_PATH="$1"

    # Ensure .app extension
    if [[ "$INPUT_PATH" != *.app ]]; then
        INPUT_PATH="${INPUT_PATH}.app"
    fi

    # Convert to absolute path
    if [[ "$INPUT_PATH" = /* ]]; then
        # Already absolute path
        APP_PATH="$INPUT_PATH"
    else
        # Relative path, resolve from current directory
        APP_PATH="$(cd "$(dirname "$INPUT_PATH")" 2>/dev/null && pwd)/$(basename "$INPUT_PATH")"
    fi
fi

# Check if app exists
if [ ! -d "$APP_PATH" ]; then
    echo "Error: App not found at: $APP_PATH"
    exit 1
fi

# Get app name and base name
APP_NAME="$(basename "$APP_PATH")"
BASE_NAME="${APP_NAME%.app}"

# Get directory where app is located
APP_DIR="$(dirname "$APP_PATH")"

# Set output paths
OUTPUT_DMG="${APP_DIR}/${BASE_NAME}.dmg"
VOLUME_NAME="Install ${BASE_NAME}"
STAGING_DIR="${APP_DIR}/dmg_staging"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}Creating DMG (simple approach)...${NC}"
echo -e "${YELLOW}  App: $APP_PATH${NC}"
echo -e "${YELLOW}  Output: $OUTPUT_DMG${NC}"

# Clean up
rm -rf "$STAGING_DIR"
rm -f "$OUTPUT_DMG"
rm -f "${APP_DIR}/temp.dmg"

# Create staging directory
echo -e "${GREEN}Step 1: Creating staging folder...${NC}"
mkdir -p "$STAGING_DIR"

# Copy app to staging
echo -e "${GREEN}Step 2: Copying app...${NC}"
cp -R "$APP_PATH" "$STAGING_DIR/"

# Bundle the Gatekeeper fix script inside the app
echo -e "${GREEN}Step 3: Bundling Gatekeeper fix script...${NC}"
RESOURCES_DIR="$STAGING_DIR/$APP_NAME/Contents/Resources"
if [ -d "$RESOURCES_DIR" ]; then
    cp "$SCRIPT_DIR/fix_gatekeeper.sh" "$RESOURCES_DIR/"
    chmod +x "$RESOURCES_DIR/fix_gatekeeper.sh"
    echo -e "${YELLOW}  ✓ fix_gatekeeper.sh bundled in app${NC}"
else
    echo -e "${YELLOW}  ⚠ Resources directory not found, skipping fix script${NC}"
fi

# Copy installation instructions to DMG
echo -e "${GREEN}Step 4: Adding installation instructions...${NC}"
cp "$SCRIPT_DIR/dmg_install_instructions.txt" "$STAGING_DIR/📖 How to Install.txt"

# Create Applications symlink
echo -e "${GREEN}Step 5: Creating Applications link...${NC}"
ln -s /Applications "$STAGING_DIR/Applications"

# Create DMG from staging folder
echo -e "${GREEN}Step 6: Creating DMG...${NC}"
hdiutil create -volname "$VOLUME_NAME" \
    -srcfolder "$STAGING_DIR" \
    -ov -format UDZO \
    "$OUTPUT_DMG"

# Clean up staging
rm -rf "$STAGING_DIR"

if [ -f "$OUTPUT_DMG" ]; then
    SIZE=$(du -h "$OUTPUT_DMG" | awk '{print $1}')
    echo ""
    echo -e "${GREEN}✓ DMG created successfully!${NC}"
    echo -e "  File: ${OUTPUT_DMG}"
    echo -e "  Size: ${SIZE}"
else
    echo -e "${RED}Error: Failed to create DMG${NC}"
    exit 1
fi
