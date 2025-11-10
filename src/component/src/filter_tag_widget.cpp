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

#include "filter_tag_widget.h"
#include <QInputDialog>
#include <QMessageBox>

// ============================================================================
// FilterTag Implementation
// ============================================================================

FilterTag::FilterTag(const QString &extension, QWidget *parent)
    : QWidget(parent), extension(extension) {
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 2, 5, 2);
    layout->setSpacing(5);

    // Extension label with background
    label = new QLabel(extension, this);
    label->setStyleSheet(
        "QLabel {"
        "    background-color: #E3F2FD;"
        "    border: 1px solid #2196F3;"
        "    border-radius: 3px;"
        "    padding: 3px 8px;"
        "    color: #1976D2;"
        "    font-weight: bold;"
        "}"
    );

    // Delete button (small X)
    deleteButton = new QPushButton("×", this);
    deleteButton->setFixedSize(20, 20);
    deleteButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #F44336;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 10px;"
        "    font-weight: bold;"
        "    font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #D32F2F;"
        "}"
    );
    deleteButton->setCursor(Qt::PointingHandCursor);

    layout->addWidget(label);
    layout->addWidget(deleteButton);

    connect(deleteButton, &QPushButton::clicked, this, &FilterTag::DeleteClicked);
}

// ============================================================================
// FilterTagWidget Implementation
// ============================================================================

FilterTagWidget::FilterTagWidget(QWidget *parent) : QWidget(parent) {
    SetupUI();
}

void FilterTagWidget::SetupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(5);

    // Scroll area for tags
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setMaximumHeight(50);
    scrollArea->setStyleSheet(
        "QScrollArea {"
        "    border: 1px solid #CCCCCC;"
        "    border-radius: 3px;"
        "    background-color: white;"
        "}"
    );

    // Container for tags
    tagsContainer = new QWidget();
    tagsLayout = new QHBoxLayout(tagsContainer);
    tagsLayout->setContentsMargins(5, 5, 5, 5);
    tagsLayout->setSpacing(5);
    tagsLayout->addStretch();

    scrollArea->setWidget(tagsContainer);

    // Add button
    addButton = new QPushButton(tr("+ Add Filter"), this);
    addButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #4CAF50;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 3px;"
        "    padding: 5px 15px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #45A049;"
        "}"
    );
    addButton->setCursor(Qt::PointingHandCursor);

    mainLayout->addWidget(scrollArea);
    mainLayout->addWidget(addButton);

    connect(addButton, &QPushButton::clicked, this, &FilterTagWidget::OnAddClicked);
}

void FilterTagWidget::SetFilter(const QString &filter) {
    Clear();

    if (filter.isEmpty()) {
        return;
    }

    // Parse filter: "*.mp4,*.avi,*.mkv" -> ["*.mp4", "*.avi", "*.mkv"]
    QStringList extList = filter.split(',', Qt::SkipEmptyParts);

    for (const QString &ext : extList) {
        QString trimmed = ext.trimmed();
        if (!trimmed.isEmpty()) {
            AddTag(trimmed);
        }
    }
}

QString FilterTagWidget::GetFilter() const {
    return extensions.join(',');
}

QStringList FilterTagWidget::GetExtensions() const {
    return extensions;
}

void FilterTagWidget::Clear() {
    // Remove all tag widgets
    QLayoutItem *item;
    while ((item = tagsLayout->takeAt(0)) != nullptr) {
        if (item->widget() && item->widget() != tagsLayout->parentWidget()) {
            delete item->widget();
        }
        delete item;
    }

    extensions.clear();
    tagsLayout->addStretch();
}

void FilterTagWidget::AddTag(const QString &extension) {
    // Normalize extension (ensure it starts with *.)
    QString normalized = extension.trimmed();
    if (!normalized.startsWith("*.")) {
        if (normalized.startsWith('.')) {
            normalized = "*" + normalized;
        } else if (normalized.startsWith('*')) {
            if (!normalized.contains('.')) {
                normalized = "*." + normalized.mid(1);
            }
        } else {
            normalized = "*." + normalized;
        }
    }

    // Check for duplicates
    if (extensions.contains(normalized, Qt::CaseInsensitive)) {
        return;
    }

    // Remove stretch before adding new tag
    QLayoutItem *stretch = tagsLayout->takeAt(tagsLayout->count() - 1);

    // Create and add tag
    FilterTag *tag = new FilterTag(normalized, this);
    connect(tag, &FilterTag::DeleteClicked, this, &FilterTagWidget::OnTagDeleted);

    tagsLayout->addWidget(tag);
    extensions.append(normalized);

    // Re-add stretch at the end
    if (stretch) {
        tagsLayout->addItem(stretch);
    } else {
        tagsLayout->addStretch();
    }

    emit FilterChanged(GetFilter());
}

void FilterTagWidget::OnAddClicked() {
    bool ok;
    QString extension = QInputDialog::getText(
        this,
        tr("Add File Extension"),
        tr("Enter file extension (e.g., mp4, avi, *.mkv):"),
        QLineEdit::Normal,
        "",
        &ok
    );

    if (ok && !extension.isEmpty()) {
        AddTag(extension);
    }
}

void FilterTagWidget::OnTagDeleted() {
    FilterTag *tag = qobject_cast<FilterTag *>(sender());
    if (!tag) {
        return;
    }

    // Remove from extensions list
    extensions.removeAll(tag->GetExtension());

    // Remove widget
    tagsLayout->removeWidget(tag);
    tag->deleteLater();

    emit FilterChanged(GetFilter());
}

void FilterTagWidget::RetranslateUi() {
    addButton->setText(tr("+ Add Filter"));
}

void FilterTagWidget::UpdateLayout() {
    tagsContainer->updateGeometry();
    scrollArea->updateGeometry();
}
