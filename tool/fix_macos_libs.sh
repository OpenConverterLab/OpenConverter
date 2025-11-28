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

# Auto-detect build directory
BUILD_DIR=""
if [ -d "build-release/OpenConverter.app" ]; then
    BUILD_DIR="build-release"
elif [ -d "build/OpenConverter.app" ]; then
    BUILD_DIR="build"
elif [ -d "../build-release/OpenConverter.app" ]; then
    BUILD_DIR="../build-release"
elif [ -d "../build/OpenConverter.app" ]; then
    BUILD_DIR="../build"
else
    echo -e "${RED}Error: OpenConverter.app not found${NC}"
    echo "Searched in: build-release/, build/, ../build-release/, ../build/"
    echo "Please build the app first with: cd src && cmake -B build && cd build && make"
    exit 1
fi

echo -e "${GREEN}Found app bundle in: $BUILD_DIR${NC}"
cd "$BUILD_DIR"

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

echo -e "${YELLOW}Step 2.5: Bundling BMF libraries (if available)...${NC}"
# Check if BMF_ROOT_PATH is set
if [ -n "$BMF_ROOT_PATH" ]; then
    # If BMF_ROOT_PATH doesn't end with /output/bmf, append it (same logic as CMake)
    if [[ ! "$BMF_ROOT_PATH" =~ output/bmf$ ]]; then
        BMF_ROOT_PATH="$BMF_ROOT_PATH/output/bmf"
    fi

    if [ -d "$BMF_ROOT_PATH" ]; then
        echo -e "${GREEN}Found BMF at: $BMF_ROOT_PATH${NC}"

        # Create lib/ subdirectory for builtin modules (BMF hardcoded path)
        mkdir -p "$APP_FRAMEWORKS/lib"
    else
        echo -e "${YELLOW}BMF_ROOT_PATH set but directory not found: $BMF_ROOT_PATH${NC}"
        echo "  Skipping BMF bundling"
        BMF_ROOT_PATH=""
    fi
else
    echo -e "${YELLOW}BMF_ROOT_PATH not set, skipping BMF bundling${NC}"
    echo "  To bundle BMF libraries, set: export BMF_ROOT_PATH=/path/to/bmf"
fi

if [ -n "$BMF_ROOT_PATH" ] && [ -d "$BMF_ROOT_PATH" ]; then

    # Copy builtin modules to lib/ subdirectory
    echo "  Copying BMF builtin modules to Frameworks/lib/..."
    for module in libbuiltin_modules.dylib libcopy_module.dylib libcvtcolor.dylib; do
        if [ -f "$BMF_ROOT_PATH/lib/$module" ]; then
            cp "$BMF_ROOT_PATH/lib/$module" "$APP_FRAMEWORKS/lib/" 2>/dev/null || true
            chmod +w "$APP_FRAMEWORKS/lib/$module" 2>/dev/null || true
            echo "    Copied: $module"
        fi
    done

    # Copy other BMF libraries to Frameworks/
    echo "  Copying BMF core libraries to Frameworks/..."
    for lib in libbmf_py_loader.dylib libbmf_module_sdk.dylib libengine.dylib libhmp.dylib _bmf.cpython-39-darwin.so _hmp.cpython-39-darwin.so; do
        if [ -f "$BMF_ROOT_PATH/lib/$lib" ]; then
            cp "$BMF_ROOT_PATH/lib/$lib" "$APP_FRAMEWORKS/" 2>/dev/null || true
            chmod +w "$APP_FRAMEWORKS/$lib" 2>/dev/null || true
            echo "    Copied: $lib"
        fi
    done

    # Copy BMF config
    if [ -f "$BMF_ROOT_PATH/BUILTIN_CONFIG.json" ]; then
        cp "$BMF_ROOT_PATH/BUILTIN_CONFIG.json" "$APP_FRAMEWORKS/" 2>/dev/null || true
        echo "    Copied: BUILTIN_CONFIG.json"
    fi

    # Bundle BMF Python package to Resources/bmf/ (named 'bmf' for Python import)
    echo "  Copying BMF Python package to Resources/bmf/..."
    rm -rf "$APP_DIR/Contents/Resources/bmf"
    cp -R "$BMF_ROOT_PATH" "$APP_DIR/Contents/Resources/bmf" 2>/dev/null || true
    echo "    Copied BMF Python package as 'bmf'"

    echo -e "${GREEN}BMF libraries bundled successfully${NC}"
fi

# Bundle Homebrew Python stdlib (required for embedded Python in BMF)
echo -e "${YELLOW}Step 2.6: Bundling Python stdlib from Homebrew...${NC}"
PYTHON_PREFIX=$(brew --prefix python@3.9 2>/dev/null || echo "")
if [ -n "$PYTHON_PREFIX" ]; then
    PYTHON_FRAMEWORK="$PYTHON_PREFIX/Frameworks/Python.framework/Versions/3.9"
    PYTHON_STDLIB="$PYTHON_FRAMEWORK/lib/python3.9"
    PYTHON_LIB="$PYTHON_FRAMEWORK/Python"

    if [ -d "$PYTHON_STDLIB" ] && [ -f "$PYTHON_LIB" ]; then
        # Create Python directory structure in Frameworks
        mkdir -p "$APP_FRAMEWORKS/Python/lib"

        # Copy libpython
        echo "  Copying libpython..."
        cp "$PYTHON_LIB" "$APP_FRAMEWORKS/Python/lib/libpython3.9.dylib" 2>/dev/null || true
        chmod +w "$APP_FRAMEWORKS/Python/lib/libpython3.9.dylib" 2>/dev/null || true

        # Copy Python stdlib (excluding site-packages and test directories to save space)
        echo "  Copying Python stdlib (this may take a moment)..."
        mkdir -p "$APP_FRAMEWORKS/Python/lib/python3.9"
        rsync -a --exclude='site-packages' --exclude='test' --exclude='tests' --exclude='__pycache__' \
            "$PYTHON_STDLIB/" "$APP_FRAMEWORKS/Python/lib/python3.9/" 2>/dev/null || true

        # Create empty site-packages directory
        mkdir -p "$APP_FRAMEWORKS/Python/lib/python3.9/site-packages"

        echo -e "${GREEN}Python stdlib bundled successfully${NC}"
        echo "  Size: $(du -sh "$APP_FRAMEWORKS/Python" | cut -f1)"
    else
        echo -e "${RED}Warning: Homebrew Python 3.9 not found at expected location${NC}"
    fi
else
    echo -e "${RED}Warning: Homebrew Python 3.9 not installed${NC}"
fi

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

# Pre-bundle libavdevice which is needed by BMF's libbuiltin_modules but not bundled by macdeployqt
if [ -f "$FFMPEG_LIB_DIR/libavdevice.59.dylib" ]; then
    if [ ! -f "$APP_FRAMEWORKS/libavdevice.59.dylib" ]; then
        echo "  Pre-copying libavdevice.59.dylib (needed by BMF builtin modules)"
        cp "$FFMPEG_LIB_DIR/libavdevice.59.dylib" "$APP_FRAMEWORKS/"
        chmod +w "$APP_FRAMEWORKS/libavdevice.59.dylib"
    fi
fi

# Iterate multiple times to catch transitive dependencies
for iteration in 1 2 3; do
    echo "Pass $iteration: Scanning for missing dependencies..."

    # Get all dylib and .so files in Frameworks folder (including BMF Python modules)
    ALL_LIBS=$(find "$APP_FRAMEWORKS" -type f \( -name "*.dylib" -o -name "*.so" \))

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
                    # @rpath - try Homebrew and BMF locations
                    dep_basename=$(basename "$dep")
                    # Search in multiple Homebrew and BMF locations
                    search_dirs=(
                        "$FFMPEG_LIB_DIR"
                        "$(brew --prefix)/lib"
                        "$(brew --prefix webp 2>/dev/null)/lib"
                        "$(brew --prefix libvpx 2>/dev/null)/lib"
                        "$(brew --prefix opus 2>/dev/null)/lib"
                        "$(brew --prefix x264 2>/dev/null)/lib"
                        "$(brew --prefix x265 2>/dev/null)/lib"
                    )
                    # Add BMF library directories if BMF_ROOT_PATH is set
                    if [ -n "$BMF_ROOT_PATH" ]; then
                        search_dirs+=("$BMF_ROOT_PATH/lib")
                    fi
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
            # For libraries in lib/ subdirectory, use the correct path
            if [[ "$lib_path" == *"/Frameworks/lib/"* ]]; then
                install_name_tool -id "@executable_path/../Frameworks/lib/$lib_name" "$lib_path" 2>/dev/null || true
            else
                install_name_tool -id "@executable_path/../Frameworks/$lib_name" "$lib_path" 2>/dev/null || true
            fi
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

# Sign all dylibs and .so files individually first
echo "  Signing individual libraries..."
find "$APP_FRAMEWORKS" -type f \( -name "*.dylib" -o -name "*.so" \) | while read lib; do
    codesign --force --sign - "$lib" 2>/dev/null || true
done

# Sign all .so files in Python stdlib
if [ -d "$APP_FRAMEWORKS/Python/lib/python3.9/lib-dynload" ]; then
    echo "  Signing Python stdlib extensions..."
    find "$APP_FRAMEWORKS/Python/lib/python3.9/lib-dynload" -type f -name "*.so" | while read lib; do
        codesign --force --sign - "$lib" 2>/dev/null || true
    done
fi

# Sign plugins
echo "  Signing plugins..."
find "$APP_DIR/Contents/PlugIns" -type f \( -name "*.dylib" -o -name "*.so" \) 2>/dev/null | while read lib; do
    codesign --force --sign - "$lib" 2>/dev/null || true
done

# Sign the main executable
echo "  Signing main executable..."
codesign --force --sign - "$APP_EXECUTABLE" 2>/dev/null || true

# Finally sign the whole app bundle
echo "  Signing app bundle..."
codesign --force --sign - "$APP_DIR" 2>&1 || {
    echo -e "${RED}Warning: App bundle signing failed, but individual components are signed.${NC}"
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
