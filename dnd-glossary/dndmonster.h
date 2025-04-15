//
// Created by arsen on 01.04.2024.
//

#ifndef DM_ASSIST_DNDMONSTER_H
#define DM_ASSIST_DNDMONSTER_H

#include "dndcreature.h"

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


#endif //DM_ASSIST_DNDMONSTER_H
