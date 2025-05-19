#include <QApplication>
#include <QPixmap>
#include <QSplashScreen>
#include "mainwindow.h"


int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QPixmap pixmap(":/splash/splash.png");
    QSplashScreen splashScreen(pixmap);
    splashScreen.show();
    splashScreen.showMessage("Loading application", Qt::AlignBottom | Qt::AlignCenter, Qt::white);
    MainWindow w;
    w.show();

    splashScreen.finish(&w);
    return a.exec();
}