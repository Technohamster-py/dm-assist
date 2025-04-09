//
// Created by arseniy on 11.10.2024.
//

#ifndef DM_ASSIST_MAINWINDOW_H
#define DM_ASSIST_MAINWINDOW_H

#include "QAction"
#include <QMainWindow>
#include <QSettings>
#include <QShortcut>
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

public slots:
    void stopAll();
    void saveSettings();
    void loadSettings();

private:
    Ui::MainWindow *ui;

    int deviceIndex = -1;

    QVector<QPlayer*> players;

    void setupPlayers();
    void setupShortcuts();
    void handlePlayerActivation(int index);
    SettingsDialog *settingsDialog = nullptr;

private slots:
    void loadConfigFile();
    void saveConfigFile();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSettings_triggered();
};


#endif //DM_ASSIST_MAINWINDOW_H
