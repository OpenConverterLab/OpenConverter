#!/bin/bash

# Script to fix library paths in OpenConverter.app for macOS distribution
# This script does what dylibbundler does: copies libraries and fixes their paths

# Don't use set -e because some install_name_tool commands may fail on certain libraries
# and that's okay - we want to continue processing other libraries

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== OpenConverter macOS Library Fixer ===${NC}"

# Check if app bundle exists
if [ ! -d "build/OpenConverter.app" ]; then
    echo -e "${RED}Error: OpenConverter.app not found in build/ directory${NC}"
    echo "Please build the app first with: cd src && cmake -B build && cd build && make"
    exit 1
fi

cd build

APP_DIR="OpenConverter.app"
APP_FRAMEWORKS="$APP_DIR/Contents/Frameworks"
APP_EXECUTABLE="$APP_DIR/Contents/MacOS/OpenConverter"

echo -e "${YELLOW}Step 1: Running macdeployqt to bundle Qt frameworks and FFmpeg libraries...${NC}"
macdeployqt "$APP_DIR" -verbose=2

echo -e "${GREEN}macdeployqt has copied all needed libraries to Frameworks folder${NC}"

echo -e "${YELLOW}Step 2: Detecting FFmpeg installation...${NC}"
if command -v brew &> /dev/null; then
    FFMPEG_PREFIX=$(brew --prefix ffmpeg@5 2>/dev/null || brew --prefix ffmpeg 2>/dev/null || echo "")
    if [ -z "$FFMPEG_PREFIX" ]; then
        echo -e "${RED}Error: FFmpeg not found via Homebrew${NC}"
        echo "Please install with: brew install ffmpeg@5"
        exit 1
    fi
    echo -e "${GREEN}Found FFmpeg at: $FFMPEG_PREFIX${NC}"
else
    echo -e "${RED}Error: Homebrew not found${NC}"
    exit 1
fi

FFMPEG_LIB_DIR="$FFMPEG_PREFIX/lib"

echo -e "${YELLOW}Step 3: Checking if dylibbundler is available...${NC}"
if ! command -v dylibbundler &> /dev/null; then
    echo -e "${YELLOW}dylibbundler not found, installing via Homebrew...${NC}"
    brew install dylibbundler
fi

echo -e "${YELLOW}Step 4: Copying missing dependencies and fixing inter-library dependencies...${NC}"
echo "This copies missing libraries and fixes paths that reference each other"

# Function to copy a library if it doesn't exist
copy_lib_if_needed() {
    local dep_name="$1"
    local source_path="$2"

    if [ ! -f "$APP_FRAMEWORKS/$dep_name" ] && [ -f "$source_path" ]; then
        echo "  Copying missing library: $dep_name"
        cp "$source_path" "$APP_FRAMEWORKS/" 2>/dev/null || true
        chmod +w "$APP_FRAMEWORKS/$dep_name" 2>/dev/null || true
        return 0
    fi
    return 1
}

# Iterate multiple times to catch transitive dependencies
for iteration in 1 2 3; do
    echo "Pass $iteration: Scanning for missing dependencies..."

    # Get all dylib files in Frameworks folder
    ALL_LIBS=$(find "$APP_FRAMEWORKS" -name "*.dylib" -type f)

    new_libs_copied=0

    # Fix each library's dependencies
    for lib_path in $ALL_LIBS; do
        lib_name=$(basename "$lib_path")

        # Skip Qt libraries (they use @rpath correctly)
        if [[ "$lib_name" == libQt* ]] || [[ "$lib_name" == Qt* ]]; then
            continue
        fi

        if [ $iteration -eq 1 ]; then
            echo "Processing $lib_name..."
        fi

        # Get all dependencies (both absolute paths and @rpath)
        # Use || true to handle cases where grep finds no matches
        all_deps=$(otool -L "$lib_path" 2>/dev/null | awk '{print $1}' | tail -n +2 || true)

        for dep in $all_deps; do
            # Skip empty lines
            if [ -z "$dep" ]; then
                continue
            fi

            # Skip if it's already using @executable_path
            if [[ "$dep" == @executable_path* ]]; then
                continue
            fi

            # Skip system libraries
            if [[ "$dep" == /usr/lib* ]] || [[ "$dep" == /System* ]]; then
                continue
            fi

            dep_name=$(basename "$dep")

            # Skip if it's the library itself
            if [ "$dep_name" == "$lib_name" ]; then
                continue
            fi

            # If dependency doesn't exist in Frameworks, try to copy it
            if [ ! -f "$APP_FRAMEWORKS/$dep_name" ]; then
                # Try to find the library in common locations
                if [[ "$dep" == /* ]]; then
                    # Absolute path - use it directly
                    if copy_lib_if_needed "$dep_name" "$dep"; then
                        new_libs_copied=$((new_libs_copied + 1))
                    fi
                elif [[ "$dep" == @rpath/* ]]; then
                    # @rpath - try Homebrew locations
                    dep_basename=$(basename "$dep")
                    # Search in multiple Homebrew locations
                    search_dirs=(
                        "$FFMPEG_LIB_DIR"
                        "$(brew --prefix)/lib"
                        "$(brew --prefix webp 2>/dev/null)/lib"
                        "$(brew --prefix libvpx 2>/dev/null)/lib"
                        "$(brew --prefix opus 2>/dev/null)/lib"
                        "$(brew --prefix x264 2>/dev/null)/lib"
                        "$(brew --prefix x265 2>/dev/null)/lib"
                    )
                    for search_dir in "${search_dirs[@]}"; do
                        if [ -z "$search_dir" ]; then
                            continue
                        fi
                        if copy_lib_if_needed "$dep_basename" "$search_dir/$dep_basename"; then
                            new_libs_copied=$((new_libs_copied + 1))
                            break
                        fi
                    done
                fi
            fi

            # Fix the path if the dependency exists in Frameworks folder
            if [ -f "$APP_FRAMEWORKS/$dep_name" ]; then
                if [ $iteration -eq 1 ]; then
                    echo "  Fixing: $dep -> @executable_path/../Frameworks/$dep_name"
                fi
                install_name_tool -change "$dep" "@executable_path/../Frameworks/$dep_name" "$lib_path" 2>/dev/null || true
            fi
        done

        # Fix the library's own ID if it's an absolute path or @rpath
        lib_id=$(otool -D "$lib_path" 2>/dev/null | tail -n +2 | head -n 1 || true)
        if [[ ! -z "$lib_id" ]] && [[ "$lib_id" != @executable_path* ]] && [[ "$lib_id" != /usr/lib* ]] && [[ "$lib_id" != /System* ]]; then
            install_name_tool -id "@executable_path/../Frameworks/$lib_name" "$lib_path" 2>/dev/null || true
        fi
    done

    if [ $new_libs_copied -eq 0 ]; then
        echo "No new libraries copied in pass $iteration, stopping."
        break
    else
        echo "Copied $new_libs_copied new libraries in pass $iteration, continuing..."
    fi
done

echo ""
echo -e "${YELLOW}Step 5: Re-signing the app bundle...${NC}"
codesign --force --deep --sign - "$APP_DIR" 2>&1 || {
    echo -e "${RED}Warning: Code signing failed. App may not run.${NC}"
}

echo ""
echo -e "${YELLOW}Step 6: Verifying library paths...${NC}"
echo -e "${GREEN}Main executable dependencies:${NC}"
otool -L "$APP_EXECUTABLE" | grep -E "libav|libsw|libx264|libx265|libvpx|libopus" || echo "  (No FFmpeg/codec libraries directly linked)"

echo ""
echo -e "${GREEN}Sample FFmpeg library dependencies (libavcodec):${NC}"
if [ -f "$APP_FRAMEWORKS/libavcodec.59.dylib" ]; then
    otool -L "$APP_FRAMEWORKS/libavcodec.59.dylib" | grep -E "libav|libsw|libx264|libx265|libvpx|libopus" | sed 's/^/  /'
fi

echo ""
echo -e "${GREEN}=== Library fixing complete! ===${NC}"
echo ""
echo -e "${YELLOW}To test the app:${NC}"
echo "  ./OpenConverter.app/Contents/MacOS/OpenConverter"
echo ""
echo -e "${YELLOW}To create a DMG:${NC}"
echo "  macdeployqt OpenConverter.app -dmg"
echo "  # DMG will be created as OpenConverter.dmg"
echo ""
echo -e "${YELLOW}To verify all dependencies are bundled:${NC}"
echo "  otool -L OpenConverter.app/Contents/MacOS/OpenConverter"
echo "  # All paths should start with @executable_path or @rpath"
