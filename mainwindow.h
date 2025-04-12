//
// Created by arseniy on 11.10.2024.
//

#ifndef DM_ASSIST_MAINWINDOW_H
#define DM_ASSIST_MAINWINDOW_H

#include "QAction"
#include <QMainWindow>
#include <QSettings>
#include <QShortcut>
#include <QTranslator>
#include "qplayerwidget.h"
#include "settingsdialog.h"


#define ORGANIZATION_NAME "Technohaster"
#define ORGANIZATION_DOMAIN "github.com/Technohamster-py"
#define APPLICATION_NAME "DM-assist"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    QString workingDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/dm_assist_files/";

    ~MainWindow() override;

    void changeLanguage(const QString &languageCode);

signals:
    void languageChanged(const QString &languageCode);

public slots:
    void stopAll();
    void stopOtherPlayers(int exeptId);
    void saveSettings();
    void loadSettings();

    void openHelp();
    void openDonate();

private:
    Ui::MainWindow *ui;
    QTranslator translator;
    QString currentLanguage;

    int deviceIndex = -1;
    QVector<QPlayer*> players;

    void setupPlayers();
    void setupShortcuts();
    SettingsDialog *settingsDialog = nullptr;
    Settings paths;

private slots:
    void loadConfigFile();
    void saveConfigFile();
    void on_actionSettings_triggered();
};


#endif //DM_ASSIST_MAINWINDOW_H
