#include "mainwindow.h"
#include "src/core/dialogs/startup-dialog/recentcampaigndialog.h"
#include "settings.h"

#include <QLoggingCategory>

int main(int argc, char *argv[]) {
    qRegisterMetaType<Status>();
    qRegisterMetaType<QList<Status>>();
    QApplication a(argc, argv);

    // QLoggingCategory::setFilterRules("*.debug=false");
    qSetMessagePattern("[%{time yyyy.MM.dd h:mm:ss.zzz ttt} %{if-debug}D%{endif}%{if-info}I%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}]:: %{if-category}%{category}: %{endif}%{if-critical}|IN %{function}| %{endif}%{if-fatal}|IN %{function}| %{endif}%{message}");

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