//
// Created by arseniy on 02.04.2025.
//

#ifndef DM_ASSIST_SETTINGSDIALOG_H
#define DM_ASSIST_SETTINGSDIALOG_H

#include <QDialog>
#include <QTreeWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class SettingsDialog; }
QT_END_NAMESPACE

class SettingsDialog : public QDialog {
Q_OBJECT

public:
    explicit SettingsDialog(QString organisationName, QString applicationName, QWidget *parent = nullptr);
    ~SettingsDialog() override;

protected:
    void loadSettings();
    void saveSettings();

private slots:
    void onTreeItemSelected(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void on_folderButton_clicked();

private:
    Ui::SettingsDialog *ui;

    QString m_organisationName;
    QString m_applicationName;
};


#endif //DM_ASSIST_SETTINGSDIALOG_H
