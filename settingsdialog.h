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

public slots:
    bool exportSettingsToFile(QString& filePath);
    bool importSettingsFromFile(QString& filePath);

protected:
    void loadSettings();
    void saveSettings();
    const Settings paths;

private slots:
    void onTreeItemSelected(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void on_applyButton_clicked();

    void on_folderButton_clicked();
    void on_themeButton_clicked();

    void exportSettings();
    void importSettings();

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

    void setupIcons();

    QStringList m_excludedKeys {
        paths.general.audioDevice,
        paths.general.lang,
        paths.general.dir,
        paths.general.defaultCampaignDir,
        paths.appearance.stretch,
        paths.session.recent,
        paths.session.campaign
    };
};


class XmlSettingsWriter : public QObject {
    Q_OBJECT
public:
    explicit XmlSettingsWriter(QObject* parent = nullptr) {};

    bool exportToFile(const QString &filePath, QSettings &settings, QString *errorString = nullptr);
    bool importFromFile(const QString& filePath, QSettings& settings, QString* errorString = nullptr);

    void excludeKeys(const QStringList& keys);
    void addExcludedKeys(const QStringList& addKeys);

private:
    QString serializeVariant(const QVariant& v) const;
    QVariant deserializeVariant(const QString& text, const QString& type) const;

    QStringList m_excludedKeys = {};
};


#endif //DM_ASSIST_SETTINGSDIALOG_H
