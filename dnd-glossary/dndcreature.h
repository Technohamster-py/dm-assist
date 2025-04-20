//
// Created by arsen on 01.04.2024.
//

#ifndef DM_ASSIST_DNDCREATURE_H
#define DM_ASSIST_DNDCREATURE_H

#include <QString>
#include <QFile>
#include <QPixmap>
#include <QMap>
#include "../base/creature.h"

class dndCreature : public Creature{
    Q_OBJECT
public:
    explicit dndCreature(QString creatureName);
    explicit dndCreature(QFile *xmlConfig);

    ~dndCreature();

    static int getBonuseFromCharacteristic(int characteristicValue);
    int getCharacterSheetId() const {return id;};
    int getAc() const {return ac;};
    int getHp() const {return hp;};
    int getMaxHp() const {return maxHP;};
    int getInitiativeBonus() const {return initiativeBonus;};

    void setMaxHp(int value);
    bool setHp(int value);
    void setAc(int value);
    void setInitiativeBonus(int value);

    void setFullHp();
    void doDamage(int damageValue);

signals:
    void imageChanged();
    void hpChanged();
    void maxHpCahged();
    void acChanged();
    void initiativeBonusChanged();

protected:
    QString notes;

    QString imagePath;
    QPixmap image;

    // Characteristics
    QMap<QString, int> characteristics;

    int initiativeBonus = 0;

    int ac = 10;
    int maxHP = 0;
    int hp = 0;
    QString hpFormula = "";

    QMap<QString, int> speedValues;
};


#endif //DM_ASSIST_DNDCREATURE_H
