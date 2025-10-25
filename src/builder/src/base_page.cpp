#include "../include/base_page.h"

BasePage::BasePage(QWidget *parent) : QWidget(parent) {}

BasePage::~BasePage() {}

void BasePage::OnPageActivated() {
    // Default implementation - can be overridden by derived classes
}

void BasePage::OnPageDeactivated() {
    // Default implementation - can be overridden by derived classes
}
