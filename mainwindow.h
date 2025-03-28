//
// Created by arseniy on 11.10.2024.
//

#ifndef DM_ASSIST_MAINWINDOW_H
#define DM_ASSIST_MAINWINDOW_H

#include <QMainWindow>
#include "qplayerwidget.h"



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

private slots:
    void loadConfigFile();
    void saveConfigFile();
};


#endif //DM_ASSIST_MAINWINDOW_H
