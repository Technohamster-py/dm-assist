#ifndef DM_ASSIST_DNDMODELS_H
#define DM_ASSIST_DNDMODELS_H

#include <QAbstractTableModel>
#include <QMap>
#include <QJsonArray>
#include <QJsonObject>

struct Attack {
    QString title = "Attack";
    QString ability = "str";
    QString damage = "1d4";
    bool prof = true;
    int bonus = +0;
    QString notes = "";

public:
    int attackBonus(QMap<QString, int> bonusMap) const{
        int profBonus = (prof) ? bonusMap["prof"] : 0;
        return bonusMap[ability] + profBonus;
    }
};

class DndAttackModel : public QAbstractTableModel{
Q_OBJECT

public:
    explicit DndAttackModel(QObject* parent = nullptr);
    DndAttackModel(const QJsonArray& attackList, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {return m_weaponList.size();};
    int columnCount(const QModelIndex &parent = QModelIndex()) const override {return 5;};
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    void addAttack(const Attack& weapon);
    void deleteAttack(int row);
    Attack getAttack(int row) const;

    bool fromJson(const QJsonArray& attackData);

public slots:
    void setStrBonus(int bonus) { m_bonusMap["str"] = bonus; emit dataChanged(index(0, 1), index(rowCount()-1, 2));};
    void setDexBonus(int bonus) { m_bonusMap["dex"] = bonus; emit dataChanged(index(0, 1), index(rowCount()-1, 2));};
    void setConBonus(int bonus) { m_bonusMap["con"] = bonus; emit dataChanged(index(0, 1), index(rowCount()-1, 2));};
    void setIntBonus(int bonus) { m_bonusMap["int"] = bonus; emit dataChanged(index(0, 1), index(rowCount()-1, 2));};
    void setWisBonus(int bonus) { m_bonusMap["wis"] = bonus; emit dataChanged(index(0, 1), index(rowCount()-1, 2));};
    void setChaBonus(int bonus) { m_bonusMap["cha"] = bonus; emit dataChanged(index(0, 1), index(rowCount()-1, 2));};
    void setProfBonus(int bonus) { m_bonusMap["prof"] = bonus; emit dataChanged(index(0, 1), index(rowCount()-1, 2));};

private:
    QVector<Attack> m_weaponList;

    QMap<QString, int> m_bonusMap = {
            {"prof", 0},
            {"str", 0},
            {"dex", 0},
            {"con", 0},
            {"int", 0},
            {"wis", 0},
            {"cha", 0},
    };
};


struct Resource{
    QString title;
    int current;
    int max;
    bool refillOnShortRest;
    bool refillOnLongRest;
};

class DndResourceModel : public QAbstractTableModel{
    Q_OBJECT
public:
    explicit DndResourceModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {return m_resourcesList.size();};
    int columnCount(const QModelIndex &parent = QModelIndex()) const override {return 5;};
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    void addResource(const Resource& resource);
    void deleteResource(int row);

    bool fromJson(const QJsonObject &resourcesData);

public slots:
    void doLongRest();
    void doShortRest();
    void slot() {qDebug() << "whfwi";};

private:
    QVector<Resource> m_resourcesList;
};

#endif //DM_ASSIST_DNDMODELS_H
