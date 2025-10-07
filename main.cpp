#include <QApplication>

#include "mainwindow.h"
#include "recentcampaigndialog.h"
#include "settings.h"

int main(int argc, char *argv[]) {
    qRegisterMetaType<Status>();
    qRegisterMetaType<QList<Status>>();
    QApplication a(argc, argv);

    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
    const Settings paths;
    int startAction = settings.value(paths.general.startAction).toInt();

    MainWindow w;
    QString path;
    switch (startAction) {
        case startActions::openLast:
            w.openCampaign();           ///< Empty parameter causes to open last saved campaign
            break;
        case startActions::showRecent:
            path = RecentCampaignDialog::getCampaignPath(w.recentCampaigns());
            w.openCampaign(path);
            break;
        default:
            break;
    }

    w.showMaximized();

    return a.exec();
}