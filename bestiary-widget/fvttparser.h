#ifndef DM_ASSIST_FVTTPARSER_H
#define DM_ASSIST_FVTTPARSER_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QMap>
#include <QJsonArray>

#include "dndbeaststructure.h"



class IFvttParser : public QObject{
    Q_OBJECT
public:
    virtual ~IFvttParser() = default;
    virtual BestiaryPageData parse(const QJsonObject& root) = 0;
    static int bonusFromStat(int statValue) {return (statValue >= 10) ? (statValue - 10) / 2 : (statValue - 11) / 2;};
protected:
    QMap<QString, QList<BestiaryItem>> descriptionSections;
    QString convertToHeader(QString type){
        if (! typeToHeader.keys().contains(type))
            return type;
        return typeToHeader[type];
    };

    QMap<QString, QString> skillTitles {
            {"acr", tr("Acrobatics")},
            {"ani", tr("Animal handling")},
            {"arc", tr("Arcana")},
            {"ath", tr("Athletics")},
            {"dec", tr("Deception")},
            {"his", tr("History")},
            {"ins", tr("Insight")},
            {"itm", tr("Intimidation")},
            {"inv", tr("Investigation")},
            {"med", tr("Medicine")},
            {"nat", tr("Nature")},
            {"prc", tr("Perception")},
            {"prf", tr("Performance")},
            {"per", tr("Persuasion")},
            {"rel", tr("Religion")},
            {"slt", tr("Sleight of hand")},
            {"ste", tr("Stealth")},
            {"sur", tr("Survival")}
    };

    QMap<QString, QString> typeToHeader {
            {"action", tr("Actions")},
            {"feat", tr("Feats")},
            {"lair", tr("Lair actions")},
            {"legendary", tr("Legendary actions")},
            {"spell", tr("Spells")},
            {"trait", tr("Traits")}
    };
};


class Fvtt10Parser: public IFvttParser{
public:
    BestiaryPageData parse(const QJsonObject& root) override;
};

class Fvtt11Parser: public IFvttParser{
public:
    BestiaryPageData parse(const QJsonObject& root) override;
};


#endif //DM_ASSIST_FVTTPARSER_H
