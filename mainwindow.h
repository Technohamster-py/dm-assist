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
    QList<QPlayer*> playerList;
    QList<QWidget*> playerWidgetsList;
    QList<QPushButton*> playButtonsList;
    QList<QPushButton*> editButtonList;

private:
    Ui::MainWindow *ui;

    void configurePlayers();

private slots:
    void loadConfigFile();
    void saveConfigFile();
};


#endif //DM_ASSIST_MAINWINDOW_H
