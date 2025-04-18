//
// Created by arsen on 01.04.2024.
//

#ifndef DM_ASSIST_DNDCREATURE_H
#define DM_ASSIST_DNDCREATURE_H

#include <QString>
#include <QFile>
#include <QPixmap>
#include <QMap>
#include <QWidget>
#include "creature.h"


////////////////////////////////////////////////////
//               QDndCreature                     //
////////////////////////////////////////////////////

class dndCreature : public Creature{
    Q_OBJECT
public:
    enum lifeStatus{
        Alive,
        Knockout,
        Dead
    };

    enum conditions{
        Blinded,
        Charmed,
        Deafened,
        Frightened,
        Grappled,
        Incapacitated,
        Invisible,
        Paralyzed,
        Petrified,
        Poisoned,
        Prone,
        Restrained,
        Unconscious
    };

    explicit dndCreature(QString creatureName);
    explicit dndCreature(QFile *xmlConfig);

    ~dndCreature();

    static int getBonusFromCharacteristic(int characteristicValue);
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

   lifeStatus m_status = lifeStatus::Alive;
   QList<conditions> m_conditions;

};


////////////////////////////////////////////////////
//               QDndCharacter                    //
////////////////////////////////////////////////////

QT_BEGIN_NAMESPACE
namespace Ui { class QDndCharacterWidget; }
QT_END_NAMESPACE

class DndCharacter : public dndCreature{
public:
    explicit DndCharacter(QString characterName);
    explicit DndCharacter(QFile *xmlConfig);

    ~DndCharacter();

    void loadFromFile(QFile *xmlConfigFile);
    void saveToFile(QString pathToConfigFile);

    /// Races from PHB
    enum Races{
        Gnome,
        Dwarf,
        Dragonborn,
        Halforc,
        Halfling,
        Halfelf,
        Tiefling,
        Human,
        Elf
    };

private:
    int m_raceId;
    QString m_race;
    int m_age;
    QString m_gender;

    int m_proficiencyBonus;

    QMap<QString, int> m_classLevel;        ///< Class - level
    QMap<QString, QString> m_subclasses;    ///< Class - subclass

    QMap<QString, int> m_skillsProficiency;    ///< skill - bonus
    QMap<QString, int> m_savesProficiency;    ///< Save throw - has proficiency

    QMap<QString, QString> m_feelings;    ///< feeling name - description

    QString m_languages; ///< list of languages

    //inventory
    //spellbook

    QMap<QString, int> m_friends; ///< Friend name - friend character id
    QMap<QString, int> m_enemies; ///< Enemy name - enemy character id

    QMap<QString, QString> m_abilities;    ///< Ability name - description

};

////////////////////////////////////////////////////
//         QDndCharacterWidget                    //
////////////////////////////////////////////////////

class QDndCharacterWidget : public QWidget {
Q_OBJECT

public:
    explicit QDndCharacterWidget(DndCharacter character, QWidget *parent = nullptr);

    ~QDndCharacterWidget() override;



private:
    Ui::QDndCharacterWidget *ui;
};


////////////////////////////////////////////////////
//               QDndMonster                      //
////////////////////////////////////////////////////

class dndMonster : public dndCreature {
Q_OBJECT
public:
    explicit dndMonster(const QString& monsterTitle);
    explicit dndMonster(QFile *xmlConfig);

    ~dndMonster();


    [[nodiscard]] QString getArea() const {return m_area;};
    [[nodiscard]] QString getDescription() const {return m_description;};
    [[nodiscard]] QString getLanguages() const {return m_languages;};

    void setArea(QString area);
    void setDescription(QString description);

    void loadFromFile(QFile *xmlConfigFile);

signals:
    void areaChanged();
    void descriptionChanged();


private:
    int m_proficiencyBonus;
    QString m_languages;
    QString m_area;
    QString m_description;

    QMap<QString, bool> m_skillsProficiency;    ///< skill - has proficiency
    QMap<QString, bool> m_savesProficiency;    ///< Save throw - has proficiency

    QMap<QString, QString> m_feelings;    ///< feeling name - description
    QMap<QString, QString> m_abilities;    ///< Ability name - description
    QMap<QString, QString> m_actions;
};

#endif //DM_ASSIST_DNDCREATURE_H
