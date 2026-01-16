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

    echo -e "${GREEN}BMF libraries bundled successfully${NC}"
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
echo -e "${YELLOW}Step 6: Copying BMF Python package to Resources...${NC}"
if [ -n "$BMF_ROOT_PATH" ] && [ -d "$BMF_ROOT_PATH" ]; then
    # Bundle BMF Python package to Resources/bmf/ (named 'bmf' for Python import)
    echo "  Copying BMF Python package to Resources/bmf/..."
    rm -rf "$APP_DIR/Contents/Resources/bmf"
    cp -R "$BMF_ROOT_PATH" "$APP_DIR/Contents/Resources/bmf" 2>/dev/null || true
    echo "    Copied BMF Python package as 'bmf'"

    # Override libs in Resources/bmf/lib with already-fixed libs from Frameworks
    echo "  Overriding libraries with fixed versions from Frameworks/..."
    BMF_RESOURCES_LIB="$APP_DIR/Contents/Resources/bmf/lib"
    if [ -d "$BMF_RESOURCES_LIB" ]; then
        for lib_path in "$BMF_RESOURCES_LIB"/*.dylib "$BMF_RESOURCES_LIB"/*.so; do
            [ -f "$lib_path" ] || continue
            lib_name=$(basename "$lib_path")

            # Check if this library exists in Frameworks (already fixed)
            if [ -f "$APP_FRAMEWORKS/$lib_name" ]; then
                echo "    Overriding: $lib_name"
                cp "$APP_FRAMEWORKS/$lib_name" "$lib_path"
            fi
        done
        cp "$APP_FRAMEWORKS/lib/*" $BMF_RESOURCES_LIB/ 2>/dev/null || true
    fi

    echo -e "${GREEN}BMF Python package copied successfully${NC}"
else
    echo -e "${YELLOW}BMF_ROOT_PATH not set, skipping BMF Python package${NC}"
fi

echo ""
echo -e "${YELLOW}Step 7: Creating wrapper launcher...${NC}"
APP_MACOS_DIR="$APP_DIR/Contents/MacOS"
REAL_EXECUTABLE="$APP_MACOS_DIR/OpenConverter.real"

# Find the launcher template script
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
LAUNCHER_TEMPLATE=""
for path in "$SCRIPT_DIR/../src/resources/macos_launcher.sh" \
            "$SCRIPT_DIR/src/resources/macos_launcher.sh" \
            "../src/resources/macos_launcher.sh" \
            "src/resources/macos_launcher.sh"; do
    if [ -f "$path" ]; then
        LAUNCHER_TEMPLATE="$path"
        break
    fi
done

# Rename the original executable and install wrapper
if [ -f "$APP_EXECUTABLE" ] && [ ! -f "$REAL_EXECUTABLE" ]; then
    echo "  Renaming OpenConverter -> OpenConverter.real"
    mv "$APP_EXECUTABLE" "$REAL_EXECUTABLE"

    # Copy wrapper script from template
    if [ -n "$LAUNCHER_TEMPLATE" ] && [ -f "$LAUNCHER_TEMPLATE" ]; then
        echo "  Copying wrapper launcher from: $LAUNCHER_TEMPLATE"
        cp "$LAUNCHER_TEMPLATE" "$APP_EXECUTABLE"
    else
        echo "  Creating wrapper launcher script..."
        cat > "$APP_EXECUTABLE" << 'EOF'
#!/bin/sh
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
export DYLD_LIBRARY_PATH="$HOME/Library/Application Support/OpenConverter/Python.framework/lib${DYLD_LIBRARY_PATH:+:$DYLD_LIBRARY_PATH}"
exec "$SCRIPT_DIR/OpenConverter.real" "$@"
EOF
    fi
    chmod +x "$APP_EXECUTABLE"
    echo -e "${GREEN}Wrapper launcher created successfully${NC}"
else
    if [ -f "$REAL_EXECUTABLE" ]; then
        echo -e "${YELLOW}Wrapper already exists (OpenConverter.real found)${NC}"
    else
        echo -e "${RED}Error: Executable not found at $APP_EXECUTABLE${NC}"
    fi
fi

echo ""
echo -e "${YELLOW}Step 8: Verifying library paths...${NC}"
echo -e "${GREEN}Main executable dependencies:${NC}"
otool -L "$REAL_EXECUTABLE" | grep -E "libav|libsw|libx264|libx265|libvpx|libopus" || echo "  (No FFmpeg/codec libraries directly linked)"

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
echo "  otool -L OpenConverter.app/Contents/MacOS/OpenConverter.real"
echo "  # All paths should start with @executable_path or @rpath"
