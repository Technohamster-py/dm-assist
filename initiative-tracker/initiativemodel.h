#ifndef DM_ASSIST_INITIATIVEMODEL_H
#define DM_ASSIST_INITIATIVEMODEL_H

#include <QAbstractTableModel>
#include <QXmlStreamWriter>
#include <QIcon>


static int evaluateExpression(const QString &expression, bool *ok = nullptr);

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

/**
 * @class InitiativeModel
 * @brief A model for managing and displaying character initiatives in a table format.
 *
 * This model inherits from QAbstractTableModel and provides functionality for managing a list of characters,
 * each represented by fields such as name, initiative, armor class (AC), hit points (HP), and max hit points.
 * It supports functionalities like sorting by initiative, saving/loading data from files, and changing
 * hit points dynamically.
 */
class InitiativeModel : public QAbstractTableModel {
Q_OBJECT

public:
    enum fields{
        name = 0,
        initiative,
        Ac,
        hp,
        maxHp,
        del
    };
    explicit InitiativeModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;


    void addCharacter(const InitiativeCharacter &character);
    void removeCharacter(int row);
    void sortByInitiative();
    void evaluateHP(int row);

    InitiativeCharacter getCharacter(int row) const;
    void setCurrentIndex(int index);
    int getCurrentIndex() const;

    bool saveToFile(const QString &filename) const;
    bool loadFromFile(const QString &filename);
    bool addFromFile(const QString &filename);

signals:
    void dataChangedExternally();

private:
    QVector<InitiativeCharacter> characters;
    int currentIndex = 0;

    QIcon m_initHeaderIcon;
    QIcon m_acHeaderIcon;
    QIcon m_hpHeaderIcon;
};


#endif //DM_ASSIST_INITIATIVEMODEL_H
