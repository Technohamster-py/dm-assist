//
// Created by arsen on 01.04.2024.
//

#include "dndmonster.h"

#include <utility>

dndMonster::dndMonster(const QString& monsterTitle) : dndCreature(monsterTitle) {
    title = monsterTitle;
}

dndMonster::dndMonster(QFile *xmlConfig) : dndCreature(xmlConfig) {

}

dndMonster::~dndMonster() {

}

void dndMonster::loadFromFile(QFile *xmlConfigFile) {
    int errCode = loadCreatureFromFile(xmlConfigFile);
    if (errCode != ErrorNone){
        return;
    }

    QDomElement monsterNode = configDom.documentElement();
    id = monsterNode.attribute("id").toInt();

    title = monsterNode.firstChildElement("name").toText().data();

    characteristics["STR"] = monsterNode.firstChildElement("stats").firstChildElement("STR").toText().data().toInt();
    characteristics["CON"] = monsterNode.firstChildElement("stats").firstChildElement("CON").toText().data().toInt();
    characteristics["DEX"] = monsterNode.firstChildElement("stats").firstChildElement("DEX").toText().data().toInt();
    characteristics["WIS"] = monsterNode.firstChildElement("stats").firstChildElement("WIS").toText().data().toInt();
    characteristics["INT"] = monsterNode.firstChildElement("stats").firstChildElement("INT").toText().data().toInt();
    characteristics["CHA"] = monsterNode.firstChildElement("stats").firstChildElement("CHA").toText().data().toInt();

    QDomNodeList skillsList = monsterNode.elementsByTagName("skill");
    for (int i = 0; i < skillsList.count(); ++i) {
        QDomElement skill = skillsList.at(i).toElement();
        m_skillsProficiency[skill.attribute("name")] = skill.toText().data().toInt();
    }

    m_languages = monsterNode.firstChildElement("languages").toText().data();
    m_proficiencyBonus = monsterNode.firstChildElement("proficiency-bonus").toText().data().toInt();

    imagePath = monsterNode.firstChildElement("image").toText().data();
    image.load(imagePath);

    QDomNodeList abilityNodeList = monsterNode.elementsByTagName("ability");
    for (int i = 0; i < abilityNodeList.count(); ++i) {
        QDomElement monsterAbility = abilityNodeList.at(i).toElement();
        QString abilityDescription = monsterAbility.toText().data();
        QString abilityTitle = monsterAbility.attribute("title");
        m_abilities[abilityTitle] = abilityDescription;
    }

    QDomNodeList actionNodeList = monsterNode.elementsByTagName("action");
    for (int i = 0; i < actionNodeList.count(); ++i) {
        QDomElement monsterAction = actionNodeList.at(i).toElement();
        QString actionTitle = monsterAction.attribute("title");
        QString actionDescription = monsterAction.toText().data();
        m_actions[actionTitle] = actionDescription;
    }
}

void dndMonster::setArea(QString area) {
    m_area = std::move(area);
    emit areaChanged();
}

void dndMonster::setDescription(QString description) {
    m_description = std::move(description);
    emit descriptionChanged();
}
