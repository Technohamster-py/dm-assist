#include <QApplication>

#include "mainwindow.h"
#include "settings.h"

int main(int argc, char *argv[]) {
    qRegisterMetaType<Status>();
    qRegisterMetaType<QList<Status>>();
    QApplication a(argc, argv);

    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
    const Settings paths;
    int startAction = settings.value(paths.general.startAction).toInt();

    MainWindow w;

    switch (startAction) {
        case startActions::openLast:
            w.openCampaign();           ///< Empty parameter causes to open last saved campaign
            break;
        default:
            break;
    }

    w.show();

    return a.exec();
}