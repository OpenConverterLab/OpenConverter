# Third-Party Notices

This file explains the licensing of the **pre-built binary packages** published
on the [Releases](https://github.com/OpenConverterLab/OpenConverter/releases)
page, and lists the third-party components bundled in them.

## Source code vs. binary packages

The OpenConverter **source code** is licensed under the
[Apache License 2.0](LICENSE).

Each **pre-built binary package** is distributed as a whole under the
**GNU General Public License, version 3**
(see [licenses/GPL-3.0.txt](licenses/GPL-3.0.txt)). This is not an extra
restriction added on top of the source license: every package copies and links
against GPL-covered parts of FFmpeg (`libx264`, `libx265`) and the LGPLv3 build
of Qt, and the copyleft terms of those components extend to the combined work.

The two statements do not conflict — they cover two different distributions:

| What you download | License that applies to it |
|---|---|
| Source code (git clone, source archive) | Apache-2.0 |
| Pre-built binary packages (Releases page) | GPL-3.0 |

If you build OpenConverter yourself against an LGPL-only FFmpeg build, the
resulting binary contains no GPL-covered codecs and the Apache-2.0 terms apply
to it.

## Components bundled in the binary packages

Which components are present depends on the platform and on the build options
used for that package.

| Component | License | Source |
|---|---|---|
| OpenConverter | Apache-2.0 (GPL-3.0 as part of the packages) | https://github.com/OpenConverterLab/OpenConverter |
| FFmpeg 5.1.x, GPL-enabled build | GPL-2.0-or-later | https://ffmpeg.org/download.html |
| libx264 | GPL-2.0-or-later | https://www.videolan.org/developers/x264.html |
| libx265 | GPL-2.0 | https://www.videolan.org/developers/x265.html |
| Qt 5 / Qt 6 (Core, Gui, Widgets, Network) | LGPL-3.0 | https://download.qt.io/ |
| BMF — Babit Multimedia Framework | Apache-2.0 | https://github.com/BabitMF/bmf |
| Real-ESRGAN model weights | BSD-3-Clause | https://github.com/xinntao/Real-ESRGAN |
| Microsoft Visual C++ runtime (Windows packages only) | Microsoft redistribution terms | https://learn.microsoft.com/cpp/windows/latest-supported-vc-redist |

Additional Python dependencies used by the AI features (PyTorch, torchvision,
BasicSR, Real-ESRGAN, numpy and the Python runtime itself) are **not** part of
the packages. They are downloaded and installed on first use, and their own
license terms are presented during that installation.

## Corresponding source

The Corresponding Source for the GPL-covered components of each package is
available at no charge:

- **OpenConverter** — https://github.com/OpenConverterLab/OpenConverter —
  use the tag or commit recorded in the release notes of the package.
- **FFmpeg and the GPL codecs** — the exact upstream build used for each
  platform is pinned in the release build workflow
  (https://github.com/OpenConverterLab/OpenConverter/blob/master/.github/workflows/build.yaml).
  The upstream sources are available from https://ffmpeg.org/download.html and
  https://www.videolan.org/developers/x264.html /
  https://www.videolan.org/developers/x265.html.
- **Qt, LGPLv3 option** — https://download.qt.io/ (matching version as shown in
  the About dialog of the application).

If any of these links stops working, please open an issue and another way to
obtain the source will be provided.

## Verbatim license texts

| License | File |
|---|---|
| Apache-2.0 | [`LICENSE`](LICENSE) |
| GPL-3.0 | [`licenses/GPL-3.0.txt`](licenses/GPL-3.0.txt) |
