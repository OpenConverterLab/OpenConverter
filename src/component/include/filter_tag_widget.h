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

#ifndef FILTER_TAG_WIDGET_H
#define FILTER_TAG_WIDGET_H

#include <QWidget>
#include <QStringList>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>

/**
 * @brief A single filter tag with delete button
 */
class FilterTag : public QWidget {
    Q_OBJECT

public:
    explicit FilterTag(const QString &extension, QWidget *parent = nullptr);
    QString GetExtension() const { return extension; }

signals:
    void DeleteClicked();

private:
    QString extension;
    QLabel *label;
    QPushButton *deleteButton;
};

/**
 * @brief Widget for managing file filter extensions with visual tags
 *
 * Displays file extensions as tags (e.g., ".mp4", ".avi") with individual delete buttons.
 * Users can add new extensions via an "Add" button.
 */
class FilterTagWidget : public QWidget {
    Q_OBJECT

public:
    explicit FilterTagWidget(QWidget *parent = nullptr);

    /**
     * @brief Set filter extensions from comma-separated string
     * @param filter Format: "*.mp4,*.avi,*.mkv" or ".mp4,.avi,.mkv"
     */
    void SetFilter(const QString &filter);

    /**
     * @brief Get filter as comma-separated string
     * @return Format: "*.mp4,*.avi,*.mkv"
     */
    QString GetFilter() const;

    /**
     * @brief Get list of extensions
     * @return List of extensions (e.g., ["*.mp4", "*.avi"])
     */
    QStringList GetExtensions() const;

    /**
     * @brief Clear all filter tags
     */
    void Clear();

    /**
     * @brief Retranslate UI text
     */
    void RetranslateUi();

signals:
    /**
     * @brief Emitted when filter changes (tag added/removed)
     */
    void FilterChanged(const QString &filter);

private slots:
    void OnAddClicked();
    void OnTagDeleted();

private:
    void SetupUI();
    void AddTag(const QString &extension);
    void UpdateLayout();

    QScrollArea *scrollArea;
    QWidget *tagsContainer;
    QHBoxLayout *tagsLayout;
    QPushButton *addButton;
    QStringList extensions;
};

#endif // FILTER_TAG_WIDGET_H
