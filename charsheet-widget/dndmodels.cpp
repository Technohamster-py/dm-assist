#include "dndmodels.h"
#include <QIcon>

DndAttackModel::DndAttackModel(QObject *parent) : QAbstractTableModel(parent) {}

DndAttackModel::DndAttackModel(const QJsonArray &attackList, QObject *parent) : DndAttackModel(parent){
    fromJson(attackList);
}

QVariant DndAttackModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole && role != Qt::DecorationRole || orientation != Qt::Horizontal)
        return QVariant();

    switch (section) {
        case 0: return tr("Title");
        case 1: return tr("Bonus");
        case 2: return tr("Damage");
        case 3: return tr("Notes");
        case 4: return tr("Delete");
        default: return QVariant();
    }
}

QVariant DndAttackModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_attackList.size())
        return QVariant();

    const Attack &w = m_attackList.at(index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::UserRole) {
        if (role == Qt::DisplayRole && index.column() == 4)
            return "❌";

        switch (index.column()) {
            case 0: return w.title;
            case 1: return w.attackBonus(m_bonusMap);
            case 2: return w.damage;
            case 3: return w.notes;
        }
    }
    return QVariant();
}

Qt::ItemFlags DndAttackModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    if (index.column() == 4)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

bool DndAttackModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || index.row() >= m_attackList.size())
        return false;

    Attack &w = m_attackList[index.row()];
    QString strVal = value.toString();

    switch (index.column()) {
        case 0: w.title = strVal; break;
        case 1: w.bonus = strVal.toInt(); break;
        case 2: w.damage = strVal; break;
        case 3: w.notes = strVal; break;
        default: return false;
    }

    emit dataChanged(index, index);
    return true;
}

void DndAttackModel::addAttack(const Attack &weapon) {
    beginInsertRows(QModelIndex(), m_attackList.size(), m_attackList.size());
    m_attackList.append(weapon);
    endInsertRows();
}

void DndAttackModel::deleteAttack(int row) {
    if (row < 0 || row >= m_attackList.size()) return;

    beginRemoveRows(QModelIndex(), row, row);
    m_attackList.removeAt(row);
    endRemoveRows();
}

Attack DndAttackModel::getAttack(int row) const {
    if (row >= 0 && row < m_attackList.size())
        return m_attackList[row];
    return {};
}

bool DndAttackModel::fromJson(const QJsonArray& attackList) {
    m_attackArray = attackList;
    for (const auto& attackVal: attackList) {
        QJsonObject attack = attackVal.toObject();
        Attack weapon;

        weapon.id = attack["id"].toString();
        weapon.title = attack["name"].toObject()["value"].toString();
        weapon.damage = attack["dmg"].toObject()["value"].toString();
        weapon.ability = attack["ability"].toString();
        weapon.prof = attack["isProf"].toBool();
        weapon.bonus = attack["modBonus"].toObject()["value"].toInt();
        weapon.notes = attack["notes"].toObject()["value"].toString();

        addAttack(weapon);
    }
    return true;
}

QJsonArray DndAttackModel::toJson() {
    int i = 0;
    for (auto & attack : m_attackList) {
        if (attack.id.isEmpty()){
            attack.id = QString("weapon-%1").arg(i);
            QJsonObject attackObj;
            attackObj["id"] = attack.id;
            attackObj["name"] = QJsonObject();
            attackObj["mod"] = QJsonObject();
            attackObj["dmg"] = QJsonObject();
            attackObj["modBonus"] = QJsonObject();
            attackObj["notes"] = QJsonObject();
            m_attackArray.append(attackObj);
        }
        for (const auto & attackVal : m_attackArray){
            QJsonObject attackObj = attackVal.toObject();
            if (attackObj["id"].toString() == attack.id){
                attackObj["name"].toObject()["value"] = attack.title;
                attackObj["dmg"].toObject()["value"] = attack.damage;
                attackObj["ability"] = attack.ability;
                attackObj["isProf"] = attack.prof;
                attackObj["modBonus"].toObject()["value"] = attack.bonus;
                attackObj["notes"].toObject()["value"] = attack.notes;
            }
        }
        ++i;
    }
    return m_attackArray;
}

void DndAttackModel::editAttack(int row, const Attack& attack) {
    if (row < 0 || row >= m_attackList.size())
        return;
    m_attackList[row] = attack;
    emit dataChanged(index(row, 0), index(row, 5));
}


DndResourceModel::DndResourceModel(QObject *parent) : QAbstractTableModel(parent) {}

QVariant DndResourceModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_resourcesList.size())
        return QVariant();
    const Resource &r = m_resourcesList.at(index.row());

    if (role == Qt::DecorationRole && index.column() == 0){
        if (r.refillOnShortRest) return QIcon(":/charSheet/shortrest.svg");
        if (r.refillOnLongRest) return QIcon(":/charSheet/longrest.svg");
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::UserRole) {
        if (role == Qt::DisplayRole && index.column() == 4)
            return "❌";

        switch (index.column()) {
            case 1: return r.title;
            case 2: return r.current;
            case 3: return r.max;
        }
    }
    return QVariant();
}

QVariant DndResourceModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole && role != Qt::DecorationRole || orientation != Qt::Horizontal)
        return QVariant();

    switch (section) {
        case 0: return tr("Refill");
        case 1: return tr("Title");
        case 2: return tr("Current");
        case 3: return tr("Max");
        case 4: return tr("Delete");
        default: return QVariant();
    }
}

Qt::ItemFlags DndResourceModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    if (index.column() == 4 || index.column() == 0)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

bool DndResourceModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || index.row() >= m_resourcesList.size())
        return false;

    Resource &r = m_resourcesList[index.row()];
    QString strVal = value.toString();

    switch (index.column()) {
        case 1: r.title = strVal; break;
        case 2: r.current = strVal.toInt(); break;
        case 3: r.max = strVal.toInt(); break;
        default: return false;
    }

    emit dataChanged(index, index);
    return true;
}

void DndResourceModel::addResource(const Resource &resource) {
    beginInsertRows(QModelIndex(), m_resourcesList.size(), m_resourcesList.size());
    m_resourcesList.append(resource);
    endInsertRows();
}

void DndResourceModel::deleteResource(int row) {
    if (row < 0 || row >= m_resourcesList.size()) return;

    beginRemoveRows(QModelIndex(), row, row);
    m_resourcesList.removeAt(row);
    endRemoveRows();
}

void DndResourceModel::doLongRest() {
    for (auto& r: m_resourcesList) {
        if (r.refillOnLongRest || r.refillOnShortRest) r.current = r.max;
    }
    emit dataChanged(index(0, 2), index(m_resourcesList.size()-1, 3));
}

void DndResourceModel::doShortRest() {
    for (auto& r: m_resourcesList) {
        if (r.refillOnShortRest) r.current = r.max;
    }
    emit dataChanged(index(0, 3), index(m_resourcesList.size()-1, 3));
}

bool DndResourceModel::fromJson(const QJsonObject &resourcesData) {
    m_resourceObject = resourcesData;
    for (const auto &key: resourcesData.keys()) {
        QJsonObject resourceObj = resourcesData[key].toObject();
        Resource resource;

        resource.key = key;
        resource.title = resourceObj["name"].toString();
        resource.current = resourceObj["current"].toInt();
        resource.max = resourceObj["max"].toInt();
        resource.refillOnShortRest = resourceObj["isShortRest"].toBool();
        resource.refillOnLongRest = resourceObj["isLongRest"].toBool();

        addResource(resource);
    }
    return false;
}

bool DndResourceModel::changeRefillMode(int row) {
    if (row >= m_resourcesList.size() || row < 0)
        return false;
    Resource &r = m_resourcesList[row];

    r.refillOnShortRest = false;
    r.refillOnLongRest = !(r.refillOnLongRest);
    emit dataChanged(index(row, 0), index(row, 1));
    return true;
}

QJsonObject DndResourceModel::toJson() {
    int i = 0;
    for (auto& resource : m_resourcesList){
        if (resource.key.isEmpty()){
            resource.key = QString("resource-%1").arg(i);
            QJsonObject newResourceObj;
            newResourceObj["id"] = resource.key;
            newResourceObj["location"] = "traits";
        }

        QJsonObject resourceObj = m_resourceObject[resource.key].toObject();
        resourceObj["name"] = resource.title;
        resourceObj["current"] = resource.current;
        resourceObj["max"] = resource.max;
        resourceObj["isShortRest"] = resource.refillOnShortRest;
        resourceObj["isLongRest"] = resource.refillOnLongRest;
        resourceObj["icon"] = resource.refillOnLongRest ? "short-rest" : "long-rest";
        ++i;
    }
    return m_resourceObject;
}
