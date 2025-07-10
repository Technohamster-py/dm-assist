#ifndef DM_ASSIST_INITIATIVEMODEL_H
#define DM_ASSIST_INITIATIVEMODEL_H

#include <QAbstractTableModel>

#include <QIcon>
#include "initiativestructures.h"


static int evaluateExpression(const QString &expression, bool *ok = nullptr);

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
        statuses,
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

    void decrementStatuses();

signals:
    void dataChangedExternally();

private:
    QVector<InitiativeCharacter> characters;
    int currentIndex = 0;

    QIcon m_characterHeaderIcon;
    QIcon m_initHeaderIcon;
    QIcon m_acHeaderIcon;
    QIcon m_hpHeaderIcon;
};


#endif //DM_ASSIST_INITIATIVEMODEL_H
