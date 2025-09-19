#ifndef DM_ASSIST_DNDCHARACTERDATA_H
#define DM_ASSIST_DNDCHARACTERDATA_H

#include <QString>
#include <QMap>
#include <QJsonArray>
#include <QJsonObject>


struct DndCharacterData{
    QString name;
    QString className;
    QString subclass;
    int level;
    int prof;
    int speed;
    int ac;
    int hp;
    int maxHp;
    QMap<QString, int> stats =  {
            {"str", 10},
            {"dex", 10},
            {"con", 10},
            {"int", 10},
            {"wis", 10},
            {"cha", 10}
    };
    QMap<QString, bool> statProf =  {
            {"str", false},
            {"dex", false},
            {"con", false},
            {"int", false},
            {"wis", false},
            {"cha", false}
    };
    QMap<QString, bool> skillsProf = {
            {"athletics", false},
            {"acrobatics", false},
            {"sleight of hand", false},
            {"stealth", false},
            {"arcana", false},
            {"history", false},
            {"investigation", false},
            {"nature", false},
            {"religion", false},
            {"animal handling", false},
            {"insight", false},
            {"medicine", false},
            {"perception", false},
            {"survival", false},
            {"deception", false},
            {"intimidation", false},
            {"performance", false},
            {"persuasion", false}
    };
    QString proficienciesHtml;
    QString traitsHtml;
    QString equipmentHtml;
    QString featuresHtml;
    QString alliesHtml;
    QString personalityHtml;
    QString backgroundHtml;
    QString questsHtml;
    QString idealsHtml;
    QString bondsHtml;
    QString flawsHtml;
    QString notes;

    QJsonArray weapons;
    QJsonObject resourcesObj;
};

#endif //DM_ASSIST_DNDCHARACTERDATA_H
