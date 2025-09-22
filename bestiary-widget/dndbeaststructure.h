#ifndef DM_ASSIST_DNDBEASTSTRUCTURE_H
#define DM_ASSIST_DNDBEASTSTRUCTURE_H

#include <QString>
#include <QMap>
#include <QObject>

struct BestiaryItem {
    QString name;
    QString description;
    QString type;
    int activationCost = 1;
};

struct BestiaryPageData {
    QString name;
    QString imgLink;
    QString ac;
    QString acCalc;
    QString hp;
    QString hpFormula;
    int prof;
    QString speed;
    QString senses;
    QString languages;
    QString damageImmunity;
    QString damageResistance;
    QString conditionImmunity;
    QString damageVulnerability;
    QMap<QString, int> abilities = {
            {"str", 10},
            {"con", 10},
            {"dex", 10},
            {"int", 10},
            {"wis", 10},
            {"cha", 10}
    };
    QMap<QString, bool> abilityProf = {
            {"str", false},
            {"con", false},
            {"dex", false},
            {"int", false},
            {"wis", false},
            {"cha", false}
    };
    QString skills;
    QString details;
    QString dangerLevel;
    QString xp;
    QString description;
};

#endif //DM_ASSIST_DNDBEASTSTRUCTURE_H
