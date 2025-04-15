//
// Created by akhomchenko on 04.04.24.
//

#ifndef DM_ASSIST_DNDCAMPAIGN_H
#define DM_ASSIST_DNDCAMPAIGN_H

#include <QString>
#include <QFile>
#include <QSharedPointer>

#include "dndcharacter.h"
#include "dndmonster.h"
#include "qplayer.h"
#include "../initiative-tracker/encounter.h"

class dndCampaign : public QObject{
    Q_OBJECT
public:
    explicit dndCampaign(QString campaignTitle);
    explicit dndCampaign(QFile *xmlConfig);

    ~dndCampaign();

    void loadFromFile(QFile *xmlFile);
    void saveToFile(QString pathToXmlFile);
    void rename(QString newTitle);
    void doLongRest();

signals:
    void titleChanged();

private:
    QString m_campaignTitle;

    QList<QSharedPointer<DndCharacter>> m_characters;
    QList<QSharedPointer<dndMonster>> m_monsters;
    QList<QSharedPointer<QPlayer>> m_players;
    QList<QSharedPointer<QPlayer>> m_musicPlayers;
    QList<QSharedPointer<Encounter>> m_encounters;
};


#endif //DM_ASSIST_DNDCAMPAIGN_H
