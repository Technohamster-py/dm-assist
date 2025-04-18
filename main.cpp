#include <QApplication>

#include "qinitiativetrackerwidget.h"


#include <QDebug>


int main(int argc, char *argv[]) {
    QApplication a(argc, argv);


    QInitiativeTrackerWidget initiativeTrackerWidget;
    initiativeTrackerWidget.show();

    return a.exec();
}