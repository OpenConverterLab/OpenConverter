# OC Project Compilation Guide

## macOS

### Prerequisites

1. **Qt 6.5+** - Install from [qt.io](https://www.qt.io/download) (e.g., to `~/Qt`)
2. **FFmpeg 5.x-7.x** - Install via Homebrew: `brew install ffmpeg`
3. **CMake 3.10+** - Install via Homebrew: `brew install cmake`

### Build Steps

```bash
cd OpenConverter
mkdir build && cd build

# Configure (adjust Qt path to your installation)
cmake ../src \
  -DCMAKE_PREFIX_PATH=~/Qt/6.5.1/macos \
  -DBMF_TRANSCODER=OFF \
  -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build . -j$(sysctl -n hw.ncpu)
```

The output is `OpenConverter.app` in the build directory.

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `ENABLE_GUI` | ON | Build with Qt GUI |
| `ENABLE_TESTS` | OFF | Build unit tests |
| `BMF_TRANSCODER` | ON | Enable BMF backend (requires BMF SDK) |
| `FFTOOL_TRANSCODER` | ON | Enable FFmpeg CLI tool backend |
| `FFMPEG_TRANSCODER` | ON | Enable FFmpeg API backend |
| `FFMPEG_ROOT_PATH` | (auto via pkg-config) | Custom FFmpeg install path |

### Release Build

For release builds, after building run the library bundling script:

```bash
cmake ../src \
  -DCMAKE_PREFIX_PATH=~/Qt/6.5.1/macos \
  -DBMF_TRANSCODER=OFF \
  -DCMAKE_BUILD_TYPE=Release

cmake --build . -j$(sysctl -n hw.ncpu)
../tool/fix_macos_libs.sh
```

---

## Windows

## 1. Install Qt 5 or Qt 6

### 1.1 Download

- [Official Website Download](https://download.qt.io/official_releases/online_installers/)

  Domestic mirrors:

  - [University of Science and Technology of China](http://mirrors.ustc.edu.cn/qtproject/)
  - [Tsinghua University](https://mirrors.tuna.tsinghua.edu.cn/qt/)

### 1.2 Installation

#### 1.2.1 Qt Account Login

Before starting the installation, you need to have a Qt account. If you don't have one, click the registration button on the page. (**This guide only covers the important steps for installing Qt.**)

#### 1.2.2 Component Selection

On the installation page, select the **Qt for Desktop Development** component and click **Next** to begin the installation.

## 2. Install and Configure FFmpeg

Supported versions are 5.x to 7.x. The following example uses version 5.1.6.

### 2.1 Download FFmpeg 5.1.6

[Download Link](https://github.com/BtbN/FFmpeg-Builds/releases/download/autobuild-2025-02-02-12-58/ffmpeg-n5.1.6-16-g6e63e49496-win64-lgpl-shared-5.1.zip)

### 2.2 Configure FFmpeg Environment

Extract the downloaded ZIP file to a suitable location. Then, add the FFmpeg directory to your system’s environment variables.

## 3. Compile the OC Project

### 3.1 Project Configuration

Open the OC project in Qt and configure it. Check the Qt compile suite and click **Configure Project**.

### 3.2 Configure FFmpeg Environment in CMake

In the `CMakeLists.txt` file, find the **FFMPEG_ROOT_PATH** variable and set the FFmpeg environment path.

### 3.3 Disable BMF Compilation

In the `CMakeLists.txt` file, locate the line:

```cmake
option(BMF_TRANSCODER "Enable BMF Transcoder" ON)
```

Change **ON** to **OFF**. If you wish to compile BMF, refer to the official [BMF installation guide for Windows](https://babitmf.github.io/docs/bmf/getting_started_yourself/install/#windows).

### 3.4 Compile the OC Project

Once everything is set up, click the **Run** button in Qt to start the compilation process.
