//
// Created by arseniy on 03.04.2024.
//

#include "encounter.h"

Encounter::Encounter(QString title) {
    m_title = title;
    m_encounterModel = new QStandardItemModel();
    m_encounterModel->setHorizontalHeaderLabels(QStringList() << "" << "Title" << "AC" << "HP");

}

Encounter::~Encounter() {

}

void Encounter::addCharacter(DndCharacter *character, int initiativeRoll, bool autoAddBonus) {
    QSharedPointer<EncounterEntity> characterEntity(new EncounterEntity(character, initiativeRoll, autoAddBonus));

    int id = entities.count();

    QList<QStandardItem *> items;
    items.append(new QStandardItem(QString::number(characterEntity.data()->getInitiativeValue())));
    items.append(new QStandardItem(characterEntity.data()->getTitle()));
    items.append(new QStandardItem(QString::number(characterEntity.data()->getAC())));

    QString hpToMaxHp = QString::number(characterEntity.data()->getHP()) + "/" + QString::number(characterEntity.data()->getMaxHp());
    items.append(new QStandardItem(hpToMaxHp));

    items.append(new QStandardItem(QString::number(characterEntity.data()->getHP())));
    items.append(new QStandardItem(QString::number(characterEntity.data()->getMaxHp())));

    items.append(new QStandardItem(QString::number(id)));

    m_encounterModel->appendRow(items);

    entities.append(characterEntity);
}

void Encounter::addMonster(dndMonster *monster, int initiativeRoll, bool autoAddBonus) {
    QSharedPointer<EncounterEntity> monsterEntity(new EncounterEntity(monster, initiativeRoll, autoAddBonus));

    int id = entities.count();


    QList<QStandardItem *> items;
    items.append(new QStandardItem(QString::number(monsterEntity.data()->getInitiativeValue())));
    items.append(new QStandardItem(monsterEntity.data()->getTitle()));
    items.append(new QStandardItem(QString::number(monsterEntity.data()->getAC())));

    QString hpToMaxHp = QString::number(monsterEntity.data()->getHP()) + "/" + QString::number(monsterEntity.data()->getMaxHp());
    items.append(new QStandardItem(hpToMaxHp));

    items.append(new QStandardItem(QString::number(monsterEntity.data()->getHP())));
    items.append(new QStandardItem(QString::number(monsterEntity.data()->getMaxHp())));

    items.append(new QStandardItem(QString::number(id)));

    m_encounterModel->appendRow(items);

    entities.push_back(monsterEntity);
}

void Encounter::setTitle(QString title) {
    m_title = title;
}


EncounterEntity::EncounterEntity(DndCharacter *entity, int initiativeRoll, bool autoAddBonus) {
    m_type = character;
    m_title = entity->getTitle();
    m_character = entity;
    m_characterSheetId = entity->getCharacterSheetId();
    m_ac = entity->getAc();
    m_hp = entity->getHp();
    m_maxHp = entity->getMaxHp();
    m_initiativeBonus = entity->getInitiativeBonus();

    setInitiativeValue(initiativeRoll, autoAddBonus);
}

EncounterEntity::EncounterEntity(dndMonster *entity, int initiativeRoll, bool autoAddBonus) {
    m_type = monster;
    m_characterSheetId = entity->getCharacterSheetId();
    m_title = entity->getTitle();
    m_ac = entity->getAc();
    m_hp = entity->getHp();
    m_maxHp = entity->getMaxHp();
    m_initiativeBonus = entity->getInitiativeBonus();

    setInitiativeValue(initiativeRoll, autoAddBonus);
}

EncounterEntity::~EncounterEntity() {

}

void EncounterEntity::setInitiativeValue(int initiativeRoll, bool autoAddBonus) {
    if(autoAddBonus)
        m_initiativeValue = initiativeRoll + m_initiativeBonus;
    else
        m_initiativeValue = initiativeRoll;
}

void EncounterEntity::setHp(int value) {
    if(value <= m_maxHp){
        if(m_type == character)
            m_character->setHp(value);
        m_hp = value;
    }
}

void EncounterEntity::setMaxHp(int value) {
    if(m_type == character)
        m_character->setMaxHp(value);
    m_maxHp = value;
}

