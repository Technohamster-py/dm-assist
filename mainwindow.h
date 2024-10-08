//
// Created by akhomchenko on 25.03.24.
//

#ifndef DM_ASSIST_MAINWINDOW_H
#define DM_ASSIST_MAINWINDOW_H

#include <QMainWindow>
#include "encounter.h"
#include "qplayerinitiativeview.h"
#include "qinitiativetrackerwidget.h"
#include "qplayer.h"



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

public slots:
    void stopAllMusic();

protected slots:
    void slotShareTracker(Encounter *encounter);

protected:
    qPlayerInitiativeView* playerInitiativeView;
    QInitiativeTrackerWidget* initiativeWidget;

    QPlayer *player1;
    QPlayer *player2;
    QPlayer *player3;
    QPlayer *player4;
    QPlayer *player5;
    QPlayer *player6;
    QPlayer *player7;
    QPlayer *player8;
    QPlayer *player9;
    QPlayer *player0;

private:
    Ui::MainWindow *ui;

    void configurePlayers();
};


#endif //DM_ASSIST_MAINWINDOW_H
