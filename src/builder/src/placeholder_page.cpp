#include "../include/placeholder_page.h"

PlaceholderPage::PlaceholderPage(const QString &title, QWidget *parent)
    : BasePage(parent), pageTitle(title) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    label = new QLabel(QString("Page: %1\n\nThis page will be implemented later.").arg(title), this);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 16px; color: #666;");

    layout->addWidget(label);
    setLayout(layout);
}

PlaceholderPage::~PlaceholderPage() {}

QString PlaceholderPage::GetPageTitle() const {
    return pageTitle;
}
