# OpenConverter - Project Knowledge

## Overview

OpenConverter is a Qt + FFmpeg-based media converter with GUI and CLI modes. It supports multiple transcoding backends: FFmpeg API, FFmpeg CLI tool (FFTool), and BMF framework.

## Build (macOS)

```bash
# Prerequisites: FFmpeg installed via Homebrew (pkg-config), Qt 6.5.1 at /Users/jacklau/Qt
mkdir build && cd build
cmake ../src -DCMAKE_PREFIX_PATH=/Users/jacklau/Qt/6.5.1/macos -DBMF_TRANSCODER=OFF -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j$(sysctl -n hw.ncpu)
# Output: build/OpenConverter.app
```

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `ENABLE_GUI` | ON | Build with Qt GUI |
| `ENABLE_TESTS` | OFF | Build unit tests (uses GTest) |
| `BMF_TRANSCODER` | ON | Enable BMF backend (requires BMF SDK) |
| `FFTOOL_TRANSCODER` | ON | Enable FFmpeg CLI tool backend |
| `FFMPEG_TRANSCODER` | ON | Enable FFmpeg API backend |
| `FFMPEG_ROOT_PATH` | (auto) | Custom FFmpeg install path |

### Dependencies

- **Qt**: 6.5.1 at `/Users/jacklau/Qt/6.5.1/macos` (Components: Core, Gui, Widgets, Network)
- **FFmpeg**: 5.1.x from Homebrew (libavcodec, libavformat, libavfilter, libavutil, libswresample, libswscale)
- **BMF** (optional): at `/Users/jacklau/Documents/Programs/Git/Github/bmf/output/bmf`

## Project Structure

```
src/
├── CMakeLists.txt          # Main build file
├── main.cpp                # Entry point (GUI or CLI based on ENABLE_GUI)
├── common/                 # Shared data structures (encode_parameter, info, stream_context)
├── engine/                 # Converter engine orchestration
├── transcoder/             # Backend implementations (ffmpeg, fftool, bmf)
├── builder/                # Qt GUI pages and logic
├── component/              # Reusable Qt widgets
├── resources/              # UI files, translations, icons
└── tests/                  # GTest-based unit tests
```

## Key Patterns

- GUI pages inherit from `base_page` and live in `builder/`
- Reusable widgets live in `component/`
- Transcoder backends implement the `transcoder` interface in `transcoder/include/transcoder.h`
- FFmpeg version detection is automatic (supports v4.x through v7.x)
- macOS release builds use `tool/fix_macos_libs.sh` to bundle libraries

## Testing

```bash
cmake ../src -DENABLE_TESTS=ON -DENABLE_GUI=OFF -DBMF_TRANSCODER=OFF
cmake --build .
ctest
```
