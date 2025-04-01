//
// Created by arseniy on 11.10.2024.
//

#ifndef DM_ASSIST_MAINWINDOW_H
#define DM_ASSIST_MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include "qplayerwidget.h"

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

    ~MainWindow() override;

public slots:
    void stopAll();
protected:
    QList<QPlayer*> playerList;             ///< Список всех плееров
    QList<QWidget*> playerWidgetsList;      ///< Список всех контейнеров для плееров
    QList<QPushButton*> playButtonsList;    ///< Список всех кнопок play
    QList<QPushButton*> editButtonList;     ///< Список всех кнопок вызова диалога изменения

    QShortcut *stopShortcut;                ///< Шорткат для остановки всех плееров

private:
    Ui::MainWindow *ui;

    void configurePlayers();
    void saveSettings();
    void loadSettings();

private slots:
    void loadConfigFile();
    void saveConfigFile();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
};


#endif //DM_ASSIST_MAINWINDOW_H
