#include "../include/base_page.h"
#include "../include/open_converter.h"
#include "../include/shared_data.h"
#include <QLineEdit>

BasePage::BasePage(QWidget *parent) : QWidget(parent) {}

BasePage::~BasePage() {}

void BasePage::OnPageActivated() {
    // Default implementation - can be overridden by derived classes
}

void BasePage::OnPageDeactivated() {
    // Default implementation - can be overridden by derived classes
}

void BasePage::HandleSharedDataUpdate(QLineEdit *inputFileLineEdit,
                                       QLineEdit *outputFileLineEdit,
                                       const QString &defaultFormat) {
    if (!inputFileLineEdit || !outputFileLineEdit) {
        return;
    }

    // Get shared data from main window
    OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
    if (!mainWindow || !mainWindow->GetSharedData()) {
        return;
    }

    SharedData *sharedData = mainWindow->GetSharedData();
    QString sharedPath = sharedData->GetInputFilePath();

    if (sharedPath.isEmpty()) {
        return;
    }

    // Check if shared path is different from current input
    if (sharedPath != inputFileLineEdit->text()) {
        // Update input field
        inputFileLineEdit->setText(sharedPath);

        // Call hook for derived class to handle input change
        OnInputFileChanged(sharedPath);

        // Reset output path to auto-generate when input changes
        sharedData->ResetOutputPath();

        // Call hook for derived class to update output path
        OnOutputPathUpdate();
    } else {
        // Input path is the same, but update output path in case format changed
        QString format = defaultFormat;
        QString outputPath = sharedData->GenerateOutputPath(format);
        outputFileLineEdit->setText(outputPath);
    }
}

void BasePage::OnInputFileChanged(const QString &newPath) {
    // Default implementation - can be overridden by derived classes
    Q_UNUSED(newPath);
}

void BasePage::OnOutputPathUpdate() {
    // Default implementation - can be overridden by derived classes
}
