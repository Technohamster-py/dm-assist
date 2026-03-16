#ifndef DM_ASSIST_RECENTCAMPAIGNDIALOG_H
#define DM_ASSIST_RECENTCAMPAIGNDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>

/**
 * @class RecentCampaignDialog
 *
 * @brief A dialog class that displays and manages recent campaigns.
 *
 * This class provides an interface to display, interact with,
 * and manage the recent campaigns the user has accessed or created.
 * It may include features such as listing recent campaigns,
 * selecting a campaign to load, or removing a campaign from the list.
 *
 */
class RecentCampaignDialog : public QDialog {
    Q_OBJECT
public:
    RecentCampaignDialog(const QStringList& recent, QWidget *parent = nullptr);

    static QString getCampaignPath(const QStringList& recent, QWidget* parent = nullptr);

private slots:
    void onItemActivated(QListWidgetItem* item);
    void onBrowseClicked();

private:
    void loadRecent();

    QListWidget* m_list;
    QPushButton* m_browseButton;
    QStringList m_recent;
};


#endif //DM_ASSIST_RECENTCAMPAIGNDIALOG_H
