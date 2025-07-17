#ifndef DM_ASSIST_STATUSMANAGER_H
#define DM_ASSIST_STATUSMANAGER_H

#include <QObject>
#include <QList>
#include "initiativestructures.h"

class StatusManager{
public:
    static StatusManager& instance();

    QList<Status> availableStatuses() const;

    void addStatus(Status status);
    void removeStatus(Status status);

    void load();
    void save();
private:
    StatusManager();
    QList<Status> m_statuses;
};


#endif //DM_ASSIST_STATUSMANAGER_H
