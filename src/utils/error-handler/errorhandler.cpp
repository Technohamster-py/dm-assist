#include "errorhandler.h"
#include <QDateTime>

quint64 ErrorHandler::lastErrorTime = 0;

void ErrorHandler::showError(const QString &title, const QString &msg) {
    quint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - lastErrorTime < 5000) return;
    lastErrorTime = now;

    QMessageBox messageBox;
    messageBox.setIcon(QMessageBox::Critical);
    messageBox.setWindowTitle(title);
    messageBox.setText(msg);
    messageBox.exec();
}
