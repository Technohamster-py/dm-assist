#ifndef DM_ASSIST_ERRORHANDLER_H
#define DM_ASSIST_ERRORHANDLER_H

#include <QMessageBox>
#include <QString>

class ErrorHandler {
public:
    static void showError(const QString& title, const QString& msg);
private:
    static quint64 lastErrorTime;
};


#endif //DM_ASSIST_ERRORHANDLER_H
