#ifndef DM_ASSIST_SETTINGSDIALOG_H
#define DM_ASSIST_SETTINGSDIALOG_H

#include <QDialog>
#include <QTreeWidget>
#include <QKeySequenceEdit>
#include "settings.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SettingsDialog; }
QT_END_NAMESPACE

/**
 * @class SettingsDialog
 * @brief A dialog interface for configuring application settings.
 *
 * The SettingsDialog class provides a graphical interface for managing various
 * application settings including audio devices, language preferences, themes,
 * and general configurations.
 *
 * The class integrates with QSettings for loading and saving user preferences
 * and utilizes a navigation tree for presenting configuration categories to the user.
 */
class SettingsDialog : public QDialog {
Q_OBJECT

public:
    explicit SettingsDialog(QString organisationName, QString applicationName, QWidget *parent = nullptr);
    ~SettingsDialog() override;

protected:
    void loadSettings();
    void saveSettings();
    Settings paths;

private slots:
    void onTreeItemSelected(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void on_applyButton_clicked();

    void on_folderButton_clicked();
    void on_themeButton_clicked();

    void onKeySequenceChanged(QKeySequence newSeq);

private:
    Ui::SettingsDialog *ui;

    QString m_organisationName;
    QString m_applicationName;

    void populateAudioDevices();
    QStringList deviceNames;
    QVector<int> deviceIndices;

    QHash<QKeySequenceEdit*, QString> m_hotkeyHash;
    bool validateKeySequences();

    void populateLanguages();
    void populateThemes();
    void populateStyles();
    void populateTokenModes();
};


#endif //DM_ASSIST_SETTINGSDIALOG_H
