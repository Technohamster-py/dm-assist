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
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog() override;

private slots:
    void onTreeItemSelected(QTreeWidgetItem *current, QTreeWidgetItem *previous);

private:
    Ui::SettingsDialog *ui;
};


#endif //DM_ASSIST_SETTINGSDIALOG_H
