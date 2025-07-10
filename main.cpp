#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[]) {
    qRegisterMetaType<Status>();
    qRegisterMetaType<QList<Status>>();
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    return a.exec();
}