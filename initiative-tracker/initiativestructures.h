#ifndef DM_ASSIST_INITIATIVESTRUCTURES_H
#define DM_ASSIST_INITIATIVESTRUCTURES_H

#include <QXmlStreamWriter>

struct Status {
    QString title = "";
    QString iconPath = "";
    int remainingRounds = 0;

    bool operator==(const Status& other) const {
        return title == other.title;
    }
};

Q_DECLARE_METATYPE(Status)
Q_DECLARE_METATYPE(QList<Status>)

static QList<QString> standardStatuses = {"blinded",
                                          "charmed",
                                          "deafened",
                                          "exhaustion",
                                          "frightened",
                                          "grappled",
                                          "incapacitated",
                                          "invisible",
                                          "paralyzed",
                                          "petrified",
                                          "poisoned",
                                          "prone",
                                          "restrained",
                                          "stunned",
                                          "unconscious"};

static  QMap<QString, QString> standardStatusIcons = {
        {"blinded",         ":/statuses/status-blinded.svg"},
        {"charmed",         ":/statuses/status-charmed.svg"},
        {"deafened",        ":/statuses/status-deafened.svg"},
        {"exhaustion",      ":/statuses/status-exhaustion.svg"},
        {"frightened",      ":/statuses/status-frightened.svg"},
        {"grappled",        ":/statuses/status-grappled.svg"},
        {"incapacitated",   ":/statuses/status-incapacitated.svg"},
        {"invisible",       ":/statuses/status-invisible.svg"},
        {"paralyzed",       ":/statuses/status-paralyzed.svg"},
        {"petrified",       ":/statuses/status-petrified.svg"},
        {"poisoned",        ":/statuses/status-poisoned.svg"},
        {"prone",           ":/statuses/status-prone.svg"},
        {"restrained",      ":/statuses/status-restrained.svg"},
        {"stunned",         ":/statuses/status-stunned.svg"},
        {"unconscious",     ":/statuses/status-unconscious.svg"}
};
/**
 * @struct InitiativeCharacter
 * @brief Represents a character with initiative, armor class, and health information.
 *
 * The InitiativeCharacter struct is used to store information about a character,
 * including its name, initiative value, armor class (AC), current health points (HP),
 * and maximum health points (MaxHP). This struct provides methods to serialize and
 * deserialize the character data using XML.
 */
struct InitiativeCharacter {
    QString name;
    int initiative;
    int ac;
    QString hp;     // может содержать выражение типа "50-20"
    int maxHp;
    QList<Status> statuses;

    void writeToXml(QXmlStreamWriter &writer) const {
        writer.writeStartElement("Character");
        writer.writeTextElement("Name", name);
        writer.writeTextElement("Initiative", QString::number(initiative));
        writer.writeTextElement("AC", QString::number(ac));
        writer.writeTextElement("HP", hp);
        writer.writeTextElement("MaxHP", QString::number(maxHp));
        writer.writeEndElement();
    }

    static InitiativeCharacter readFromXml(QXmlStreamReader &reader) {
        InitiativeCharacter character;
        while (!(reader.tokenType() == QXmlStreamReader::EndElement && reader.name() == QString("Character"))) {
            if (reader.readNextStartElement()) {
                if (reader.name() == QString("Name")) character.name = reader.readElementText();
                else if (reader.name() == QString("Initiative")) character.initiative = reader.readElementText().toInt();
                else if (reader.name() == QString("AC")) character.ac = reader.readElementText().toInt();
                else if (reader.name() == QString("HP")) character.hp = reader.readElementText();
                else if (reader.name() == QString("MaxHP")) character.maxHp = reader.readElementText().toInt();
                else reader.skipCurrentElement();
            }
        }
        return character;
    }
};

#endif //DM_ASSIST_INITIATIVESTRUCTURES_H
