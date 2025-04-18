//
// Created by arsen on 01.04.2024.
//

#include "dndcreature.h"
#include <utility>
#include <QTextStream>

#include "ui_qdndcharacterwidget.h"


////////////////////////////////////////////////////
//               QDndCreature                     //
////////////////////////////////////////////////////

dndCreature::dndCreature(QString creatureName) : Creature(creatureName){
    title = std::move(creatureName);
}

dndCreature::dndCreature(QFile *xmlConfig) : Creature(xmlConfig){
}

dndCreature::~dndCreature() {

}

int dndCreature::getBonusFromCharacteristic(int characteristicValue) {
    return (characteristicValue >= 0) ? (characteristicValue / 10) : (characteristicValue - 10 + 1) / 10;
}

void dndCreature::setMaxHp(int value) {
    maxHP = value;
    if (hp > maxHP)
        hp = maxHP;
    emit maxHpCahged();
}

bool dndCreature::setHp(int value) {
    if(value <= maxHP){
        hp = value;
        emit hpChanged();
        return true;
    } else
        return false;
}

void dndCreature::setFullHp() {
    hp = maxHP;
}

void dndCreature::doDamage(int damageValue) {
    int newHp = hp - damageValue;
    if (newHp < 0)
        newHp = 0;
    setHp(newHp);
}

void dndCreature::setAc(int value) {
    ac = value;
    emit acChanged();
}

void dndCreature::setInitiativeBonus(int value) {
    initiativeBonus = value;
    emit initiativeBonusChanged();
}


////////////////////////////////////////////////////
//               QDndCharacter                    //
////////////////////////////////////////////////////

DndCharacter::DndCharacter(QString characterName) : dndCreature(characterName) {
    title = characterName;
}

DndCharacter::DndCharacter(QFile *xmlConfig) : dndCreature(xmlConfig) {
    loadFromFile(xmlConfig);
}

DndCharacter::~DndCharacter() {

}

void DndCharacter::loadFromFile(QFile *xmlConfigFile) {
    int errCode = loadCreatureFromFile(xmlConfigFile);
    if (errCode != ErrorNone){
        return;
    }

    QDomElement characterNode = configDom.documentElement();
    id = characterNode.attribute("id").toInt();

    title = characterNode.firstChildElement("name").toText().data();

    QDomElement raceNode = characterNode.firstChildElement("race");
    m_raceId = raceNode.attribute("id").toInt();
    m_race = raceNode.toText().data();

    m_age = characterNode.firstChildElement("age").toText().data().toInt();

    characteristics["STR"] = characterNode.firstChildElement("stats").firstChildElement("STR").toText().data().toInt();
    characteristics["CON"] = characterNode.firstChildElement("stats").firstChildElement("CON").toText().data().toInt();
    characteristics["DEX"] = characterNode.firstChildElement("stats").firstChildElement("DEX").toText().data().toInt();
    characteristics["WIS"] = characterNode.firstChildElement("stats").firstChildElement("WIS").toText().data().toInt();
    characteristics["INT"] = characterNode.firstChildElement("stats").firstChildElement("INT").toText().data().toInt();
    characteristics["CHA"] = characterNode.firstChildElement("stats").firstChildElement("CHA").toText().data().toInt();

    QDomNodeList skillsList = characterNode.elementsByTagName("skill");
    for (int i = 0; i < skillsList.count(); ++i) {
        QDomElement skill = skillsList.at(i).toElement();
        m_skillsProficiency[skill.attribute("name")] = skill.toText().data().toInt();
    }

    QDomNodeList classesList = characterNode.elementsByTagName("class");
    for (int i = 0; i < classesList.count(); ++i) {
        QDomElement characterClass = classesList.at(i).toElement();
        QString className = characterClass.toText().data();
        int level = characterClass.attribute("level").toInt();
        QString subclass = characterClass.attribute("subclass");
        m_classLevel[className] = level;
        m_subclasses[className] = subclass;
    }

    ac = characterNode.firstChildElement("ac").toText().data().toInt();
    hp = characterNode.firstChildElement("hp").attribute("current").toInt();
    maxHP = characterNode.firstChildElement("hp").attribute("max").toInt();

    QDomNodeList feelingsList = characterNode.elementsByTagName("feeling");
    for (int i = 0; i < feelingsList.count(); i++){
        QString feelingName = feelingsList.at(i).toElement().attribute("name");
        QString feelingDescription = feelingsList.at(i).toElement().toText().data();
        m_feelings[feelingName] = feelingDescription;
    }

    m_languages = characterNode.firstChildElement("languages").toText().data();

    m_proficiencyBonus = characterNode.firstChildElement("proficiency-bonus").toText().data().toInt();

    imagePath = characterNode.firstChildElement("image").toText().data();
    image.load(imagePath);
}

void DndCharacter::saveToFile(QString pathToConfigFile) {
    configDom.clear();

    QDomElement characterNode = configDom.createElement("character");
    characterNode.setAttribute("id", id);
    configDom.appendChild(characterNode);

    QDomElement nameNode = configDom.createElement("name");
    nameNode.setNodeValue(title);
    characterNode.appendChild(nameNode);

    QDomElement raceNode = configDom.createElement("race");
    raceNode.setAttribute("id", m_raceId);
    raceNode.setNodeValue(m_race);
    characterNode.appendChild(raceNode);

    QDomElement ageNode = configDom.createElement("age");
    ageNode.setNodeValue(QString::number(m_age));
    characterNode.appendChild(ageNode);

    QDomElement genderNode = configDom.createElement("gender");
    genderNode.setNodeValue(m_gender);
    characterNode.appendChild(genderNode);

    QDomElement imageNode = configDom.createElement("image");
    imageNode.setNodeValue(imagePath);
    characterNode.appendChild(imageNode);

    QDomElement acNode = configDom.createElement("ac");
    acNode.setNodeValue(QString::number(ac));
    characterNode.appendChild(acNode);

    QDomElement hpNode = configDom.createElement("hp");
    hpNode.setAttribute("max", maxHP);
    hpNode.setAttribute("current", hp);
    characterNode.appendChild(hpNode);

    QDomElement classesNode = configDom.createElement("classes");
    characterNode.appendChild(classesNode);
    for (int i = 0; i < m_classLevel.keys().count(); ++i) {
        QString className = m_classLevel.keys().at(i);
        QDomElement classNode = configDom.createElement("class");
        classNode.setNodeValue(className);
        classNode.setAttribute("level", m_classLevel[className]);
        classNode.setAttribute("subclass", m_subclasses[className]);
        classesNode.appendChild(classNode);
    }

    QDomElement statsNode = configDom.createElement("stats");
    characterNode.appendChild(statsNode);
    for (int i = 0; i < characteristics.keys().count(); ++i) {
        QString characteristicName = characteristics.keys().at(i);
        QDomElement statNode = configDom.createElement(characteristicName);
        statNode.setNodeValue(QString::number(characteristics[characteristicName]));
        statsNode.appendChild(statNode);
    }

    QDomElement skillsNode = configDom.createElement("skills");
    characterNode.appendChild(skillsNode);
    for (int i = 0; i < m_skillsProficiency.keys().count(); ++i) {
        QString skillName = m_skillsProficiency.keys().at(i);
        QDomElement skillNode = configDom.createElement("skill");
        skillNode.setAttribute("name", skillName);
        skillNode.setNodeValue(QString::number(m_skillsProficiency[skillName]));
        skillsNode.appendChild(skillNode);
    }

    QDomElement savesNode = configDom.createElement("saving-throws");
    characterNode.appendChild(savesNode);
    for (int i = 0; i < m_savesProficiency.keys().count(); ++i) {
        QString saveName = m_savesProficiency.keys().at(i);
        QDomElement saveNode = configDom.createElement("throw");
        savesNode.setAttribute("characteristic", saveName);
        savesNode.setNodeValue(QString::number(m_savesProficiency[saveName]));
        savesNode.appendChild(saveNode);
    }

    QDomElement languagesNode = configDom.createElement("languages");
    languagesNode.setNodeValue(m_languages);
    characterNode.appendChild(languagesNode);

    QDomElement proficiencyNode = configDom.createElement("proficiency-bonus");
    proficiencyNode.setNodeValue(QString::number(m_proficiencyBonus));
    characterNode.appendChild(proficiencyNode);
}


////////////////////////////////////////////////////
//         QDndCharacterWidget                    //
////////////////////////////////////////////////////

QDndCharacterWidget::QDndCharacterWidget(DndCharacter character, QWidget *parent) :
        QWidget(parent), ui(new Ui::QDndCharacterWidget) {
    ui->setupUi(this);
}

QDndCharacterWidget::~QDndCharacterWidget() {
    delete ui;
}


////////////////////////////////////////////////////
//               QDndMonster                      //
////////////////////////////////////////////////////

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