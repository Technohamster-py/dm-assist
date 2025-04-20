//
// Created by arseniy on 19.04.2025.
//

#ifndef DM_ASSIST_INITIATIVEMODEL_H
#define DM_ASSIST_INITIATIVEMODEL_H

#include <QAbstractTableModel>
#include <QXmlStreamWriter>

static int evaluateExpression(const QString &expression, bool *ok = nullptr);

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
        while (!(reader.tokenType() == QXmlStreamReader::EndElement && reader.name() == "Character")) {
            if (reader.readNextStartElement()) {
                if (reader.name() == "Name") character.name = reader.readElementText();
                else if (reader.name() == "Initiative") character.initiative = reader.readElementText().toInt();
                else if (reader.name() == "AC") character.ac = reader.readElementText().toInt();
                else if (reader.name() == "HP") character.hp = reader.readElementText();
                else if (reader.name() == "MaxHP") character.maxHp = reader.readElementText().toInt();
                else reader.skipCurrentElement();
            }
        }
        return character;
    }
};

class InitiativeModel : public QAbstractTableModel {
Q_OBJECT

public:
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

    /**
     * @brief Сохраняет текущее состояние модели в XML-файл.
     * @param filename Путь к файлу
     * @return true, если успешно сохранено
     */
    bool saveToFile(const QString &filename) const;
    /**
     * @brief Загружает состояние модели из XML-файла.
     * @param filename Путь к файлу
     * @return true, если успешно загружено
     */
    bool loadFromFile(const QString &filename);

signals:
    void dataChangedExternally();

private:
    QVector<InitiativeCharacter> characters;
    int currentIndex = 0;

};


#endif //DM_ASSIST_INITIATIVEMODEL_H
