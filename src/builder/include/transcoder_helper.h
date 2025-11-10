/*
 * Copyright 2025 Jack Lau
 * Email: jacklau1222gm@gmail.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TRANSCODER_HELPER_H
#define TRANSCODER_HELPER_H

#include <QString>

class QWidget;

/**
 * @brief Helper class to get current transcoder name from main window
 *
 * This class provides a centralized way to retrieve the currently selected
 * transcoder name from the OpenConverter main window. It eliminates code
 * duplication across pages and batch processing.
 *
 * Usage:
 *   // From a page (QWidget)
 *   QString transcoder = TranscoderHelper::GetCurrentTranscoderName(this);
 *
 *   // From any QObject with parent widget
 *   QString transcoder = TranscoderHelper::GetCurrentTranscoderName(parentWidget);
 */
class TranscoderHelper {
public:
    /**
     * @brief Get current transcoder name from main window
     * @param widget Any widget in the application (page, dialog, etc.)
     * @return Current transcoder name (e.g., "FFMPEG", "BMF", "FFTOOL")
     *         Returns "FFMPEG" as default if main window not found
     */
    static QString GetCurrentTranscoderName(QWidget *widget);
};

#endif // TRANSCODER_HELPER_H
