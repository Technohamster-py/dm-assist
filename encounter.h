//
// Created by arseniy on 03.04.2024.
//

#ifndef DM_ASSIST_ENCOUNTER_H
#define DM_ASSIST_ENCOUNTER_H

#include "dndcreature.h"
#include <QStandardItemModel>
#include <QSharedPointer>

class EncounterEntity{
public:
    explicit EncounterEntity(DndCharacter *entity, int initiativeRoll = 0, bool autoAddBonus = false);
    explicit EncounterEntity(dndMonster *entity, int initiativeRoll = 0, bool autoAddBonus = false);

    ~EncounterEntity();

    enum Type{
        monster = 0,
        character = 1
    };

    [[nodiscard]] int getInitiativeValue() const {return m_initiativeValue;};
    [[nodiscard]] int getAC() const {return m_ac;};
    [[nodiscard]] int getHP() const {return m_hp;};
    [[nodiscard]] int getMaxHp() const {return m_maxHp;};
    QString getTitle() const {return m_title;};

    void setHp(int value);
    void setMaxHp(int value);
    void setInitiativeValue(int initiativeRoll, bool autoAddBonus);

private:
    int m_type;
    DndCharacter *m_character;

    QString m_title = "";
    int m_characterSheetId = 0;
    int m_initiativeBonus = 0;
    int m_initiativeValue = 0;
    int m_ac = 10;
    int m_hp = 0;
    int m_maxHp = 0;
};


class Encounter {
public:
    Encounter(QString title);
    ~Encounter();

    QList<QSharedPointer<EncounterEntity>> entities;

    QString getTitle() const {return m_title;};
    QStandardItemModel* getModel() const {return m_encounterModel;};

    void setTitle(QString title);

    void addCharacter(DndCharacter *character, int initiativeRoll = 0, bool autoAddBonus = false);
    void addMonster(dndMonster *monster, int initiativeRoll = 0, bool autoAddBonus = false);

private:
    QStandardItemModel *m_encounterModel;

    QString m_title = "";
};


#endif //DM_ASSIST_ENCOUNTER_H
