// recentcampaigndialog.cpp
#include "recentcampaigndialog.h"
#include <QSettings>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>

RecentCampaignDialog::RecentCampaignDialog(const QStringList &recent, QWidget *parent) {
    setWindowTitle(tr("Выбор кампании"));
    resize(500, 400);

    QVBoxLayout *layout = new QVBoxLayout(this);
    m_list = new QListWidget(this);
    layout->addWidget(m_list);

    m_browseButton = new QPushButton(tr("Открыть другую..."), this);
    layout->addWidget(m_browseButton);


    connect(m_list, &QListWidget::itemActivated, this, &RecentCampaignDialog::onItemActivated);
    connect(m_browseButton, &QPushButton::clicked, this, &RecentCampaignDialog::onBrowseClicked);

    loadRecent();
}

void RecentCampaignDialog::loadRecent(){
    m_list->clear();
    for (const QString &path : m_recent) {
        QListWidgetItem *item = new QListWidgetItem(path);
        item->setToolTip(path);
        m_list->addItem(item);
    }

    if (m_list->count() > 0)
        m_list->setCurrentRow(0);
}

void RecentCampaignDialog::onItemActivated(QListWidgetItem *item) {
    if (!item) return;
    accept();
}

void RecentCampaignDialog::onBrowseClicked() {
    QString path = QFileDialog::getExistingDirectory(this, tr("Open campaign"), QString());
    if (!path.isEmpty()) {
        m_list->insertItem(0, path);
        m_list->setCurrentRow(0);
    }
}



QString RecentCampaignDialog::getCampaignPath(const QStringList& recent, QWidget *parent) {
    RecentCampaignDialog dlg(recent, parent);
    if (dlg.exec() == QDialog::Accepted && dlg.m_list->currentItem())
        return dlg.m_list->currentItem()->text();
    return QString();
}
