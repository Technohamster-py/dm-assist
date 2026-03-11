#include "recentcampaigndialog.h"
#include <QSettings>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>

RecentCampaignDialog::RecentCampaignDialog(const QStringList &recent, QWidget *parent) : QDialog(parent), m_recent(recent){
    setWindowTitle(tr("Choose campaign"));
    resize(500, 400);

    QVBoxLayout *layout = new QVBoxLayout(this);
    m_list = new QListWidget(this);
    layout->addWidget(m_list);

    m_browseButton = new QPushButton(tr("Open another..."), this);
    layout->addWidget(m_browseButton);


    connect(m_list, &QListWidget::itemActivated, this, &RecentCampaignDialog::onItemActivated);
    connect(m_browseButton, &QPushButton::clicked, this, &RecentCampaignDialog::onBrowseClicked);

    loadRecent();
}

/**
 * @brief Populates the recent campaign list in the dialog.
 *
 * Clears the current campaign list, then iterates through the provided
 * list of recent campaigns (`m_recent`) to populate the GUI list widget
 * (`m_list`) with items. For each campaign path, a corresponding
 * `QListWidgetItem` is created and added to the list. The tooltip of each
 * list item is also set to the campaign's path.
 *
 * If the list of recent campaigns is not empty, the first item is
 * automatically selected.
 */
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


/**
 * @brief Opens a dialog to select a recent campaign and retrieves its path.
 *
 * This method creates an instance of `RecentCampaignDialog` initialized with
 * a list of recent campaign paths and a parent widget. It displays the dialog
 * modally and checks if the user has accepted the dialog's selection. If a campaign
 * is selected, the method returns the corresponding path; otherwise, it returns
 * an empty string.
 *
 * @param recent A list of file paths representing recent campaigns shown in the dialog.
 * @param parent The parent widget for the dialog. Defaults to `nullptr` if not specified.
 * @return The file path of the selected campaign if an item was chosen; otherwise, an empty string.
 */
QString RecentCampaignDialog::getCampaignPath(const QStringList& recent, QWidget *parent) {
    RecentCampaignDialog dlg(recent, parent);
    if (dlg.exec() == QDialog::Accepted && dlg.m_list->currentItem())
        return dlg.m_list->currentItem()->text();
    return QString();
}
