#ifndef DM_ASSIST_FVTTPARSER_H
#define DM_ASSIST_FVTTPARSER_H

#include <QJsonArray>

#include "dndbeaststructure.h"


/**
 * @class IFvttParser
 * @brief Interface for parsing Foundry VTT bestiary files.
 *
 * This interface defines the methods required for parsing FVTT files
 * into a structured format that can be utilized by applications.
 *
 * The implementation of this interface is responsible for handling syntax,
 * structure, and semantics defined by the FVTT specification.
 *
 * The interface does not itself perform any parsing logic. It is intended to
 * be implemented by classes that require FVTT parsing functionality.
 */
class IFvttParser : public QObject{
    Q_OBJECT
public:
    virtual ~IFvttParser() = default;
    virtual BestiaryPageData parse(const QJsonObject& root) = 0;

    /**
     * @brief Computes the ability score bonus based on a given stat value.
     *
     * This function calculates the modifier associated with an ability score
     * according to the standard rules, which define that an ability score of
     * 10 or 11 is a +0 modifier, with higher scores increasing the bonus and
     * lower scores decreasing it.
     *
     * @param statValue The ability score value, typically ranging from 1 to 20.
     * @return The computed ability score bonus based on the input stat value.
     */
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


/**
 * @class Fvtt10Parser
 * @brief Implementation of the IFvttParser interface for handling Foundry VTT 10 format bestiary files.
 *
 * This class specializes in parsing JSON data from Foundry Virtual Tabletop (FVTT) version 10 bestiary files.
 * The parser processes the JSON object and converts it into a `BestiaryPageData` structure used by the application.
 *
 * The `Fvtt10Parser` class overrides the `parse` function to provide specific parsing logic for the FVTT 10 data format.
 * It is designed to be dynamically used wherever an `IFvttParser` interface is required for file parsing.
 */
class Fvtt10Parser: public IFvttParser{
public:
    BestiaryPageData parse(const QJsonObject& root) override;
};

/**
 * @class Fvtt11Parser
 * @brief Implementation of the IFvttParser interface for parsing Foundry VTT 11 format bestiary files.
 *
 * This class provides functionality to parse Foundry VTT (FVTT) bestiary files
 * adhering to the version 11 specification into structured data. The resulting
 * structured data is encapsulated in the BestiaryPageData object.
 *
 * Fvtt11Parser is used to handle files with a specific version or format marker,
 * commonly identified within the input JSON data. It takes care of processing
 * and interpreting features specific to format 11 of Foundry VTT files.
 *
 * The `parse` method overrides the virtual base implementation provided by
 * IFvttParser and is the primary point of interaction for this parser.
 */
class Fvtt11Parser: public IFvttParser{
public:
    BestiaryPageData parse(const QJsonObject& root) override;
};


#endif //DM_ASSIST_FVTTPARSER_H
