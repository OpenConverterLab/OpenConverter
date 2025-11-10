# Component Directory

## Overview

The `src/component/` directory contains reusable UI components that are shared across multiple pages in OpenConverter. These components are designed to be independent, well-tested, and provide consistent UI/UX.

## Directory Structure

```
src/component/
├── include/           # Public headers for components
│   ├── batch_input_widget.h
│   ├── batch_output_widget.h
│   ├── bitrate_widget.h
│   ├── codec_selector_widget.h
│   ├── file_selector_widget.h
│   ├── filter_tag_widget.h
│   ├── format_selector_widget.h
│   ├── pixel_format_widget.h
│   ├── progress_widget.h
│   ├── quality_widget.h
│   ├── resolution_widget.h
│   └── simple_video_player.h
└── src/               # Component implementations
    ├── batch_input_widget.cpp
    ├── batch_output_widget.cpp
    ├── bitrate_widget.cpp
    ├── codec_selector_widget.cpp
    ├── file_selector_widget.cpp
    ├── filter_tag_widget.cpp
    ├── format_selector_widget.cpp
    ├── pixel_format_widget.cpp
    ├── progress_widget.cpp
    ├── quality_widget.cpp
    ├── resolution_widget.cpp
    └── simple_video_player.cpp
```

## Current Components

### Encoding Parameter Widgets

#### 1. ResolutionWidget
- **Purpose**: Width x Height selection with "auto" support
- **Header**: `src/component/include/resolution_widget.h`
- **Implementation**: `src/component/src/resolution_widget.cpp`
- **Used By**: CompressPicturePage, CreateGifPage, TranscodePage

#### 2. PixelFormatWidget
- **Purpose**: Pixel format selection with "auto" option
- **Header**: `src/component/include/pixel_format_widget.h`
- **Implementation**: `src/component/src/pixel_format_widget.cpp`
- **Used By**: CompressPicturePage, TranscodePage

#### 3. BitrateWidget
- **Purpose**: Bitrate selection with "auto" support
- **Header**: `src/component/include/bitrate_widget.h`
- **Implementation**: `src/component/src/bitrate_widget.cpp`
- **Used By**: TranscodePage (video and audio)

#### 4. QualityWidget
- **Purpose**: Quality/qscale selection for image and video encoding
- **Header**: `src/component/include/quality_widget.h`
- **Implementation**: `src/component/src/quality_widget.cpp`
- **Used By**: CompressPicturePage (can be used in TranscodePage)

#### 5. CodecSelectorWidget
- **Purpose**: Codec selection with "auto" and "copy" options
- **Header**: `src/component/include/codec_selector_widget.h`
- **Implementation**: `src/component/src/codec_selector_widget.cpp`
- **Used By**: Can be used in TranscodePage, ExtractAudioPage

#### 6. FormatSelectorWidget
- **Purpose**: Output format selection for video, audio, and image
- **Header**: `src/component/include/format_selector_widget.h`
- **Implementation**: `src/component/src/format_selector_widget.cpp`
- **Used By**: Can be used in RemuxPage, ExtractAudioPage, CompressPicturePage

### UI Infrastructure Widgets

#### 7. FileSelectorWidget
- **Purpose**: Unified file selection widget (input/output, single/batch)
- **Header**: `src/component/include/file_selector_widget.h`
- **Implementation**: `src/component/src/file_selector_widget.cpp`
- **Used By**: ALL pages for file selection

#### 8. ProgressWidget
- **Purpose**: Progress bar + label for conversion tracking
- **Header**: `src/component/include/progress_widget.h`
- **Implementation**: `src/component/src/progress_widget.cpp`
- **Used By**: ALL conversion pages (RemuxPage, TranscodePage, ExtractAudioPage, CutVideoPage, CompressPicturePage)

#### 9. FilterTagWidget
- **Purpose**: Visual tag-based file filter management
- **Header**: `src/component/include/filter_tag_widget.h`
- **Implementation**: `src/component/src/filter_tag_widget.cpp`
- **Used By**: BatchFileDialog

#### 10. BatchInputWidget
- **Purpose**: Batch input file selection with directory support
- **Header**: `src/component/include/batch_input_widget.h`
- **Implementation**: `src/component/src/batch_input_widget.cpp`
- **Used By**: Batch processing system

#### 11. BatchOutputWidget
- **Purpose**: Batch output configuration (directory, suffix, naming)
- **Header**: `src/component/include/batch_output_widget.h`
- **Implementation**: `src/component/src/batch_output_widget.cpp`
- **Used By**: ALL pages with batch support

#### 12. SimpleVideoPlayer
- **Purpose**: FFmpeg-based video player widget
- **Header**: `src/component/include/simple_video_player.h`
- **Implementation**: `src/component/src/simple_video_player.cpp`
- **Used By**: CutVideoPage (for video preview and time selection)

## Usage

### Including Components

Since `src/component/include/` is added to the include path in CMakeLists.txt, you can include components directly:

```cpp
#include "resolution_widget.h"
#include "pixel_format_widget.h"
#include "bitrate_widget.h"
```

### Example

```cpp
// In your page header
#include "resolution_widget.h"

class MyPage : public BasePage {
    Q_OBJECT
private:
    ResolutionWidget *resolutionWidget;
};

// In your page implementation
void MyPage::SetupUI() {
    resolutionWidget = new ResolutionWidget(this);
    layout->addWidget(resolutionWidget);
}

void MyPage::CreateEncodeParameter() {
    int width = resolutionWidget->GetWidth();
    int height = resolutionWidget->GetHeight();
    // Use width and height...
}
```

## Design Principles

### 1. Independence
- Components should not depend on specific pages
- Components should be self-contained and reusable
- Components should have minimal external dependencies

### 2. Consistency
- All components follow the same API patterns
- All components support "auto" mode (0 or empty string)
- All components provide `RetranslateUi()` for internationalization

### 3. Simplicity
- Components encapsulate complex UI logic
- Components provide simple getter/setter APIs
- Components emit signals for reactive UI updates

### 4. Documentation
- All components have comprehensive header documentation
- All components have usage examples in `doc/REUSABLE_WIDGETS.md`
- All components follow OpenConverter coding conventions

## Adding New Components

When adding a new component to this directory:

1. **Create header** in `src/component/include/`
   - Follow naming convention: `*_widget.h`
   - Add Apache 2.0 license header
   - Document all public methods

2. **Create implementation** in `src/component/src/`
   - Follow naming convention: `*_widget.cpp`
   - Add Apache 2.0 license header
   - Include header: `#include "your_widget.h"`

3. **Update CMakeLists.txt**
   - Add to `GUI_SOURCES` list: `${CMAKE_SOURCE_DIR}/component/src/your_widget.cpp`
   - Add to `GUI_HEADERS` list: `${CMAKE_SOURCE_DIR}/component/include/your_widget.h`

4. **Document the component**
   - Add section to `doc/REUSABLE_WIDGETS.md`
   - Include usage examples
   - Document all features and API

5. **Test the component**
   - Build and test in isolation
   - Test in at least one page
   - Verify translation support

## Benefits of Component Directory

✅ **Clear separation**: Components are separate from page-specific code
✅ **Easy to find**: All reusable components in one place
✅ **Reusability**: Components can be used across any page
✅ **Maintainability**: Fix bugs once, applies everywhere
✅ **Testability**: Components can be tested independently
✅ **Scalability**: Easy to add new components as needed

## Quick Usage Examples

### Encoding Parameter Widgets

```cpp
// Resolution
ResolutionWidget *resolution = new ResolutionWidget(this);
resolution->SetResolution(1920, 1080);
int width = resolution->GetWidth();   // 1920
int height = resolution->GetHeight(); // 1080

// Pixel Format
PixelFormatWidget *pixFmt = new PixelFormatWidget(PixelFormatWidget::Video, this);
pixFmt->SetPixelFormat("yuv420p");
QString format = pixFmt->GetPixelFormat(); // "yuv420p" or "" for auto

// Bitrate
BitrateWidget *bitrate = new BitrateWidget(BitrateWidget::Video, this);
bitrate->SetBitrate(5000); // 5000 kbps
int kbps = bitrate->GetBitrate();

// Quality
QualityWidget *quality = new QualityWidget(QualityWidget::Image, this);
quality->SetQuality(5);
int qscale = quality->GetQuality();

// Codec
CodecSelectorWidget *codec = new CodecSelectorWidget(CodecSelectorWidget::VideoCodec, this);
codec->SetCodec("libx264");
QString codecName = codec->GetCodec(); // "" for auto

// Format
FormatSelectorWidget *format = new FormatSelectorWidget(FormatSelectorWidget::Video, false, this);
format->SetFormat("mp4");
QString formatName = format->GetFormat();
```

### UI Infrastructure Widgets

```cpp
// File Selector
FileSelectorWidget *input = new FileSelectorWidget(
    tr("Input File"), FileSelectorWidget::InputFile,
    tr("Select a file..."), tr("Video Files (*.mp4);;All Files (*.*)"),
    tr("Select Video"), this
);
connect(input, &FileSelectorWidget::FileSelected, this, &Page::OnFileSelected);
QString path = input->GetFilePath();

// Progress Widget
ProgressWidget *progress = new ProgressWidget(this);
progress->Show();
progress->GetProgressBar()->setValue(50);
progress->GetProgressLabel()->setText("Processing...");

// Batch Output
BatchOutputWidget *batchOutput = new BatchOutputWidget(this);
batchOutput->SetOutputSuffix("-converted");
QString outputPath = batchOutput->GenerateOutputPath(inputPath, "mp4");
```

## Future Components

Potential components to add:

- **TimeRangeWidget**: For start/end time selection (used in CutVideoPage)
- **FrameRateWidget**: For FPS selection (used in CreateGifPage)
- **PresetSelectorWidget**: For encoder preset selection (ultrafast, fast, medium, slow, etc.)
