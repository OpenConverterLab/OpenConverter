# OpenConverter - Project Knowledge

## Overview

OpenConverter is a Qt + FFmpeg-based media converter with GUI and CLI modes. It supports multiple transcoding backends: FFmpeg API, FFmpeg CLI tool (FFTool), and BMF framework.

**Key Features:** video/audio transcoding, lossless remuxing, media info display, image compression, audio extraction, video cutting, GIF creation, batch processing with queue management, AI upscaling (via BMF + Real-ESRGAN), runtime transcoder switching, Chinese/English i18n.

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
- **GTest**: submodule at `3rd_party/gtest`

## Project Structure

```
src/
├── CMakeLists.txt          # Main build configuration
├── main.cpp                # Entry point (GUI or CLI based on ENABLE_GUI / argc)
├── common/                 # Shared data structures
│   ├── include/
│   │   ├── encode_parameter.h  # Encoding settings (codec, bitrate, resolution, pixel format, time range, AI params)
│   │   ├── process_parameter.h # Progress tracking + observer pattern
│   │   ├── process_observer.h  # Observer interface for progress updates
│   │   ├── stream_context.h    # FFmpeg stream context wrapper
│   │   ├── info.h              # Media file info extraction (QuickInfo struct)
│   │   └── logger.h            # Singleton av_log-based file logger
│   └── src/                    # Corresponding .cpp files
├── engine/                 # Conversion orchestration
│   ├── include/converter.h     # Converter class: transcoder selection + convert_format()
│   └── src/converter.cpp
├── transcoder/             # Backend implementations
│   ├── include/
│   │   ├── transcoder.h         # Abstract base class (virtual transcode())
│   │   ├── transcoder_ffmpeg.h  # FFmpeg API backend
│   │   ├── transcoder_fftool.h  # FFmpeg CLI wrapper backend
│   │   └── transcoder_bmf.h     # BMF framework backend (AI upscaling)
│   └── src/                    # Corresponding .cpp files
├── builder/                # Qt GUI pages and page-level logic
│   ├── include/
│   │   ├── open_converter.h    # QMainWindow + ProcessObserver, page navigation, drag-drop
│   │   ├── base_page.h         # Abstract base for all pages (OnPageActivated/Deactivated, RetranslateUi)
│   │   ├── shared_data.h       # Shared file paths across pages
│   │   ├── converter_runner.h  # Thread-based conversion helper with progress UI
│   │   ├── transcoder_helper.h # Static helper to get current transcoder name
│   │   ├── batch_item.h        # Single batch queue item (input/output/path/status/params)
│   │   ├── batch_queue.h       # Singleton batch queue manager (thread-safe)
│   │   ├── batch_queue_dialog.h # Batch processing monitor dialog
│   │   ├── batch_file_dialog.h  # Multi-file selection dialog
│   │   ├── batch_mode_helper.h  # Coordinator between single-file and batch modes
│   │   ├── python_manager.h     # Embedded Python runtime installer/manager
│   │   ├── transcode_page.h     # Full transcoding page
│   │   ├── remux_page.h         # Container format change page
│   │   ├── cut_video_page.h     # Video cutting with player and time selection
│   │   ├── extract_audio_page.h # Audio extraction page
│   │   ├── compress_picture_page.h # Image compression page
│   │   ├── create_gif_page.h    # GIF creation page
│   │   ├── info_view_page.h     # Media info display page
│   │   ├── ai_processing_page.h # AI upscaling page (BMF-dependent)
│   │   └── placeholder_page.h   # Placeholder for disabled features
│   └── src/                    # Corresponding .cpp files + open_converter.ui
├── component/              # Reusable Qt widgets (12 widgets)
│   ├── include/
│   │   ├── file_selector_widget.h    # Single/batch file selection
│   │   ├── filter_tag_widget.h       # Visual tag-based file filter
│   │   ├── progress_widget.h         # Progress bar + label
│   │   ├── resolution_widget.h       # Width x Height with "auto"
│   │   ├── pixel_format_widget.h     # Pixel format selector
│   │   ├── bitrate_widget.h          # Bitrate selector with "auto"
│   │   ├── quality_widget.h          # Quality/qscale selector
│   │   ├── codec_selector_widget.h   # Codec selector with "auto"/"copy"
│   │   ├── format_selector_widget.h  # Output format selector
│   │   ├── batch_input_widget.h      # Batch file input with directory support
│   │   ├── batch_output_widget.h     # Batch output config (directory, suffix)
│   │   ├── simple_video_player.h     # FFmpeg-based video player (QLabel subclass)
│   │   └── python_install_dialog.h   # Python installation progress dialog (BMF)
│   └── src/                    # Corresponding .cpp files
├── resources/              # Qt resources
│   ├── lang.qrc                # Resource file (lang_chinese.qm + logo)
│   ├── lang_chinese.ts         # Chinese translations (Qt Linguist)
│   ├── requirements.txt        # Python deps for AI features
│   ├── OpenConverter.icns      # macOS app icon
│   └── linglong.yaml           # Deepin Linglong package config
├── modules/                # Python modules for BMF AI processing
│   ├── enhance_module.py       # Real-ESRGAN upscaling module
│   └── weights/                # AI model weights (.pth files, gitignored)
└── tests/                  # GTest-based unit tests
    ├── CMakeLists.txt
    └── transcoder_test.cpp
```

Root-level files:
- `.clang-format` — BasedOnStyle: LLVM, 4-space indent, -4 access modifier offset
- `.gitignore` — Ignores build dirs, .vscode, .DS_Store, AI model weights
- `.gitmodules` — GTest submodule at `3rd_party/gtest`
- `.augment-guidelines` — Detailed developer reference (superset of CLAUDE.md)
- `CONTRIBUTING.md` — AngularJS commit conventions, PR workflow

## Architecture

### Core Classes

| Class | Module | Role |
|-------|--------|------|
| `Converter` | engine | Orchestrates transcoding; creates/selecets transcoder backend |
| `Transcoder` | transcoder | Abstract base; virtual `transcode(input, output)` |
| `TranscoderFFmpeg` | transcoder | FFmpeg C API backend (most flexible) |
| `TranscoderFFTool` | transcoder | Wraps `ffmpeg` CLI binary |
| `TranscoderBMF` | transcoder | BMF framework backend (AI upscaling support) |
| `EncodeParameter` | common | All encoding settings: codec, bitrate, resolution, pixel fmt, time range, AI mode |
| `ProcessParameter` | common | Progress + time estimation; observer notification |
| `ProcessObserver` | common | Interface: `on_process_update()`, `on_time_update()` |
| `Info` / `QuickInfo` | common | Media file probing via FFmpeg |
| `StreamContext` | common | FFmpeg AVFormatContext + stream indices wrapper |
| `Logger` | common | Singleton; installs `av_log` callback; file logging toggle |

### Design Patterns

- **Observer Pattern**: `ProcessParameter` notifies `ProcessObserver` implementations (including `OpenConverter` main window)
- **Strategy Pattern**: `TranscoderFFmpeg` / `TranscoderFFTool` / `TranscoderBMF` are interchangeable
- **Factory Pattern**: `Converter::set_transcoder()` creates the right backend by name
- **Page Pattern**: GUI uses `QStackedWidget` + `BasePage` subclasses; navigation via `QButtonGroup`
- **Singleton**: `Logger`, `BatchQueue`

### GUI Architecture

- `OpenConverter` (QMainWindow) owns `QStackedWidget` for page switching and `QButtonGroup` for nav
- All pages inherit from `BasePage`: `GetPageTitle()`, `OnPageActivated()`, `OnPageDeactivated()`, `RetranslateUi()`
- `SharedData` transfers file paths between pages; auto-generates output path pattern: `input-oc-output.ext`
- `ConverterRunner` encapsulates thread-based conversion with progress/button management
- `BatchQueue` (singleton) manages queue of `BatchItem` objects with status tracking

### GUI Pages

| Page | Purpose |
|------|---------|
| `InfoViewPage` | Display media metadata (codec, resolution, bitrate, etc.) |
| `TranscodePage` | Full video/audio transcoding with codec/quality control |
| `RemuxPage` | Change container format without re-encoding |
| `CutVideoPage` | Video cutting with embedded player and time selection |
| `ExtractAudioPage` | Extract audio track from video |
| `CompressPicturePage` | Image compression with format/quality/resolution control |
| `CreateGifPage` | GIF creation from video or image sequence |
| `AIProcessingPage` | AI upscaling via Real-ESRGAN (requires BMF + Python) |

## Coding Style

### Naming Conventions (CRITICAL — per-directory differences)

**`src/common/`, `src/engine/`, `src/builder/`, `src/component/`:**
- Functions/Methods: `PascalCase` (`SetTranscoder`, `GetVideoCodec`, `OnButtonClicked`)
- Classes: `PascalCase` (`Converter`, `EncodeParameter`)
- Members/locals: `camelCase` (`videoCodec`, `inputFilePath`)

**`src/transcoder/`:**
- Functions/Methods: `snake_case` (`transcode`, `get_video_codec`)
- Classes: `PascalCase` (`TranscoderFFmpeg`)
- Members/locals: `snake_case` (`input_path`, `codec_name`)

### Other Conventions

- **Indentation**: 4 spaces (no tabs). BasedOnStyle: LLVM in `.clang-format`
- **Braces**: Opening brace on same line
- **Qt signals**: `PascalCase` (`PositionChanged`). **Qt slots**: `On` + `PascalCase` (`OnButtonClicked`)
- **FFmpeg headers**: Wrap in `extern "C" { }`
- **FFmpeg error style**: Combine assignment and check: `if ((ret = avcodec_open2(...)) < 0)`
- **FFmpeg cleanup**: Use `goto end` pattern for error-path cleanup
- **Threading**: Long operations in `QThread::create()`; UI updates via `QMetaObject::invokeMethod(Qt::QueuedConnection)`
- **License headers**: Apache 2.0 for most files; LGPL 2.1 for `src/transcoder/`
- **File endings**: No trailing whitespace, single newline at EOF (enforced by pre-commit)

## Key Patterns

### Threading Pattern for Conversions

```cpp
QThread *thread = QThread::create([this, encodeParam, processParam]() {
    Converter converter;
    bool success = converter.Convert(encodeParam, processParam);
    QMetaObject::invokeMethod(this, [this, success]() {
        OnConversionFinished(success);
    }, Qt::QueuedConnection);
});
connect(thread, &QThread::finished, thread, &QThread::deleteLater);
thread->start();
```

### Translation Workflow

```bash
cd src
lupdate -recursive builder/ component/ -ts resources/lang_chinese.ts   # Extract strings
# Manually edit .ts file translations
lrelease resources/lang_chinese.ts -qm resources/lang_chinese.qm       # Compile to binary
```

- Always store inline QLabel pointers as member variables for retranslation support
- Never create `new QLabel(tr("Text"), this)` without storing the pointer

### Adding a New GUI Page

1. Header in `src/builder/include/your_page.h`, impl in `src/builder/src/your_page.cpp`
2. Inherit from `BasePage`; implement `GetPageTitle()`, `OnPageActivated()`, `OnPageDeactivated()`
3. Add to `CMakeLists.txt` in `GUI_SOURCES` and `GUI_HEADERS`
4. Register in `OpenConverter::InitializePages()`
5. Add nav button in `open_converter.ui` or `open_converter.cpp`

### Adding a New Transcoder Backend

1. Create `transcoder/include/transcoder_yourname.h` and `transcoder/src/transcoder_yourname.cpp`
2. Inherit from `Transcoder`; implement `transcode(input_path, output_path)`
3. Add to `Converter::set_transcoder()` switch statement
4. Update `CMakeLists.txt` with conditional compilation

## CLI Mode

When `argc > 1`, `main()` routes to `handleCLI()` (no Qt dependency). Supports:
- Positional args: input file, output file (with overwrite confirmation)
- `--transcoder` (FFMPEG/BMF/FFTOOL), `-v`/`-a` codecs, `-q` qscale, `-b:v`/`-b:a` bitrates, `-pix_fmt`, `-scale WxH`
- `-ss` start time, `-to` end time, `-t` duration (HH:MM:SS or seconds)
- `-upscale N` for AI upscaling (requires BMF)

## CI/CD

GitHub Actions workflows in `.github/workflows/`:
- **`ci.yml`**: PR/push CI on Linux (FFmpeg 5.1 prebuilt) and macOS (Homebrew FFmpeg@5 + BMF from source)
- **`build.yaml`**: Multi-platform release builds (Linux x86_64/aarch64 with AppImage, macOS arm64 DMG, Windows x64, Linglong packages); uploads artifacts on tag push
- **`lint.yaml`**: pre-commit hooks (trailing-whitespace, end-of-file-fixer, codespell, YAML check)

## Tool Scripts

| Script | Purpose |
|--------|---------|
| `tool/fix_macos_libs.sh` | Bundle Qt/FFmpeg/BMF dylibs into .app, fix rpaths, code sign |
| `tool/create_dmg_simple.sh` | Create macOS DMG from .app bundle |
| `tool/fix_whitespace.sh` | Remove trailing whitespace, ensure single newline at EOF |
| `tool/format.sh` | Run clang-format (version 16.0.6) |
| `tool/download_models.sh` | Download AI model weights |
| `tool/build_and_package_macos.sh` | Full macOS build + package pipeline |
| `tool/fix_gatekeeper.sh` | Fix macOS Gatekeeper quarantine flags |
| `tool/setup_test_deps.sh` | Download test media files |

## Testing

```bash
cmake ../src -DENABLE_TESTS=ON -DENABLE_GUI=OFF -DBMF_TRANSCODER=OFF
cmake --build .
ctest
```

- Single test executable: `transcoder_test` (GTest-based)
- Links against `OpenConverterCore` static library and `gtest_main`
- Downloads test media from GitHub Releases at `src/build/tests/media/`

## Logger

`common/include/logger.h` / `common/src/logger.cpp`

- Singleton: `Logger::Instance()`
- `SetLogPath(std::string)` — set before enabling; called from `open_converter.cpp` using Qt-computed path
- `SetEnabled(bool)` — installs/restores `av_log` callback, opens/closes file in append mode
- Log path: `QStandardPaths::GenericDataLocation + "/OpenConverter/openconverter.log"`
  - macOS: `~/Library/Application Support/OpenConverter/openconverter.log`
- Persisted via `QSettings` key `logging/fileLoggingEnabled` (default: false)
- Toggle UI: **Settings → Enable Log File** in the menu bar
- Note: `av_log_get_default_callback()` not available in FFmpeg 5.1 — use `av_log_default_callback` directly

## Python / AI Processing (BMF-dependent)

- `PythonManager` (`builder/`) handles embedded Python 3.9 runtime download/install into app bundle
- `enhance_module.py` (`modules/`) is a BMF module using Real-ESRGAN for video upscaling
- `requirements.txt`: torchvision<=0.12.0, basicsr==1.4.2, realesrgan==0.3.0, numpy<2
- Model weights downloaded at build time or first run; stored in `modules/weights/`
- MPS (Apple Silicon GPU) supported for inference
- Debug mode uses system Python; Release mode bundles standalone Python.framework