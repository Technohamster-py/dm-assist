#include "updatechecker.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>

UpdateBannerWidget::UpdateBannerWidget(QWidget *parent, const QString &versionUrl, const QString &currentVersion,
                                       const QString &latestVersion)
        : QWidget(parent), m_versionUrl(versionUrl), m_currentVersion(currentVersion), m_latestVersion(latestVersion) {

    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(R"(
        UpdateBannerWidget {
            background-color: orange;
            border: 1px solid #E6B800;
            padding: 6px;
        }
        QLabel{
            color: black;
        }
        QPushButton {
            padding: 4px 8px;
            palette(text);
        }
    )");
    auto *layout = new QHBoxLayout(this);

    label = new QLabel(tr("New version available! Currently using version %1 and the latest is: %2").arg(m_currentVersion, m_latestVersion));
    auto *btnOpen = new QPushButton(tr("Download"));
    auto *btnDismiss = new QPushButton("❌");

    btnOpen->setStyleSheet("padding: 4px;");
    btnDismiss->setFixedWidth(24);

    layout->addWidget(label);
    layout->addStretch();
    layout->addWidget(btnOpen);
    layout->addWidget(btnDismiss);

    connect(btnDismiss, &QPushButton::clicked, this, &UpdateBannerWidget::onDismiss);
    connect(btnOpen, &QPushButton::clicked, this, &UpdateBannerWidget::onOpenReleasePage);
}

void UpdateBannerWidget::onDismiss() {
    this->hide();
    emit dismissed();
}

void UpdateBannerWidget::onOpenReleasePage() {
    QDesktopServices::openUrl(QUrl(m_versionUrl));
}

void UpdateBannerWidget::updateLabel() {
    label->setText(tr("New version available! Currently using version %1 and the latest is: %2").arg(m_currentVersion, m_latestVersion));
}

