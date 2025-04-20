//
// Created by arsen on 01.04.2024.
//

#include "dndcreature.h"
#include <utility>
#include <QTextStream>

dndCreature::dndCreature(QString creatureName) : Creature(creatureName){
    title = std::move(creatureName);
}

dndCreature::dndCreature(QFile *xmlConfig) : Creature(xmlConfig){
}


dndCreature::~dndCreature() {

}

int dndCreature::getBonuseFromCharacteristic(int characteristicValue) {
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
