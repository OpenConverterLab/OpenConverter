#!/bin/bash

# Script to build and package OpenConverter for macOS locally
# Usage: ./build_and_package_macos.sh

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}OpenConverter macOS Build & Package${NC}"
echo -e "${BLUE}========================================${NC}"

# Get script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_ROOT"

# Check if FFmpeg is installed
if ! command -v ffmpeg &> /dev/null; then
    echo -e "${RED}Error: FFmpeg not found!${NC}"
    echo -e "${YELLOW}Please install FFmpeg 5:${NC}"
    echo -e "  brew install ffmpeg@5"
    exit 1
fi

# Check if Qt is installed
if ! command -v qmake &> /dev/null; then
    echo -e "${RED}Error: Qt not found!${NC}"
    echo -e "${YELLOW}Please install Qt 5:${NC}"
    echo -e "  brew install qt@5"
    echo -e "  export PATH=\"\$(brew --prefix qt@5)/bin:\$PATH\""
    exit 1
fi

# Set up environment
export PATH="$(brew --prefix ffmpeg@5)/bin:$PATH"
export CMAKE_PREFIX_PATH="$(brew --prefix qt@5):$CMAKE_PREFIX_PATH"
export QT_DIR="$(brew --prefix qt@5)/lib/cmake/Qt5"
export PATH="$(brew --prefix qt@5)/bin:$PATH"

echo -e "${GREEN}✓ FFmpeg found: $(ffmpeg -version | head -n 1)${NC}"
echo -e "${GREEN}✓ Qt found: $(qmake -version | grep "Using Qt version")${NC}"

# Clean previous build
if [ -d "src/build" ]; then
    echo -e "${YELLOW}Cleaning previous build...${NC}"
    rm -rf src/build
fi

# Build
echo -e "${GREEN}Building OpenConverter...${NC}"
cd src
cmake -B build \
    -DFFMPEG_ROOT_PATH="$(brew --prefix ffmpeg@5)" \
    -DBMF_TRANSCODER=OFF

cd build
make -j$(sysctl -n hw.ncpu)

echo -e "${GREEN}✓ Build completed${NC}"

# Fix libraries
echo -e "${GREEN}Fixing macOS library paths...${NC}"
cd ..
chmod +x ../tool/fix_macos_libs.sh
../tool/fix_macos_libs.sh

echo -e "${GREEN}✓ Libraries fixed${NC}"

# Create DMG
echo -e "${GREEN}Creating DMG...${NC}"
cd ../..
chmod +x tool/create_dmg_simple.sh
tool/create_dmg_simple.sh src/build/OpenConverter.app

echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}✓ Build and packaging completed!${NC}"
echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}DMG location: ${PROJECT_ROOT}/src/build/OpenConverter.dmg${NC}"
echo -e "${YELLOW}You can now test the DMG by opening it:${NC}"
echo -e "  open src/build/OpenConverter.dmg"
