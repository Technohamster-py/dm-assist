//
// Created by arsen on 01.04.2024.
//

#ifndef DM_ASSIST_DNDCHARACTER_H
#define DM_ASSIST_DNDCHARACTER_H

#include "dndcreature.h"

class DndCharacter : public dndCreature{
public:
    explicit DndCharacter(QString characterName);
    explicit DndCharacter(QFile *xmlConfig);

    ~DndCharacter();

    void loadFromFile(QFile *xmlConfigFile);
    void saveToFile(QString pathToConfigFile);

    /// Races from PHB
    enum Races{
        Gnome = 0,
        Dwarf = 1,
        Dragonborn = 2,
        Halforc = 3,
        Halfling = 4,
        Halfelf = 5,
        Tiefling = 6,
        Human = 7,
        Elf = 8
    };

private:
    int m_raceId;
    QString m_race;
    int m_age;
    QString m_gender;

    int m_proficiencyBonus;

    QMap<QString, int> m_classLevel;        ///< Class - level
    QMap<QString, QString> m_subclasses;    ///< Class - subclass

    QMap<QString, int> m_skillsProficiency;    ///< skill - bonus
    QMap<QString, int> m_savesProficiency;    ///< Save throw - has proficiency

    QMap<QString, QString> m_feelings;    ///< feeling name - description

    QString m_languages; ///< list of languages

    //inventory
    //spellbook

    QMap<QString, int> m_friends; ///< Friend name - friend character id
    QMap<QString, int> m_enemies; ///< Enemy name - enemy character id

    QMap<QString, QString> m_abilities;    ///< Ability name - description

};


#endif //DM_ASSIST_DNDCHARACTER_H
