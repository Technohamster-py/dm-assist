#include "statusmanager.h"
#include <QSettings>
#include "../settings.h"

StatusManager::StatusManager() {
    load();
}

void StatusManager::addStatus(Status status) {
    status.remainingRounds = 0;
    for (auto &s : m_statuses) {
        if (status == s)
        {
            s.iconPath = status.iconPath;
            save();
            return;
        }
    }

    m_statuses.append(status);

    std::sort(m_statuses.begin(), m_statuses.end(), [](const Status &a, const Status &b)
    {return a.title.toLower() < b.title.toLower();});

    save();
}

StatusManager &StatusManager::instance() {
    static StatusManager inst;
    return inst;
}

void StatusManager::load() {
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
    Settings paths;
    int count = settings.beginReadArray(paths.initiative.customStatuses);
    m_statuses.clear();
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        Status s;
        s.title = settings.value(paths.initiative.customStatusTitle).toString();
        s.iconPath = settings.value(paths.initiative.customStatusIcon).toString();
        m_statuses.append(s);
    }
    settings.endArray();

    std::sort(m_statuses.begin(), m_statuses.end(), [](const Status &a, const Status &b)
    {return a.title.toLower() < b.title.toLower();});
}

void StatusManager::save() {
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
    Settings paths;
    settings.beginWriteArray(paths.initiative.customStatuses);

    for (int j = 0; j < m_statuses.size(); ++j) {
        settings.setArrayIndex(j);
        settings.setValue(paths.initiative.customStatusTitle, m_statuses[j].title);
        settings.setValue(paths.initiative.customStatusIcon, m_statuses[j].iconPath);
    }
    settings.endArray();
}

void StatusManager::removeStatus(Status status) {
    m_statuses.erase(std::remove_if(m_statuses.begin(), m_statuses.end(), [&](const Status& s){return status == s;}), m_statuses.end());
    save();
}

QList<Status> StatusManager::availableStatuses() const {
    return m_statuses;
}
