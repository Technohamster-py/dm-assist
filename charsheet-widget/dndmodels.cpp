#include "dndmodels.h"
#include <themediconmanager.h>

DndAttackModel::DndAttackModel(QObject *parent) : QAbstractTableModel(parent) {}

DndAttackModel::DndAttackModel(const QJsonArray &attackList, QObject *parent) : DndAttackModel(parent){
    fromJson(attackList);
}

QVariant DndAttackModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole && role != Qt::DecorationRole || orientation != Qt::Horizontal)
        return {};

    switch (section) {
        case fields::title: return tr("Title");
        case fields::bonus: return tr("Bonus");
        case fields::damage: return tr("Damage");
        case fields::notes: return tr("Notes");
        case fields::del: return tr("Delete");
        default: return {};
    }
}

QVariant DndAttackModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_attackList.size())
        return {};

    const Attack &w = m_attackList.at(index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::UserRole) {
        if (role == Qt::DisplayRole && index.column() == fields::del)
            return "❌";

        switch (index.column()) {
            case fields::title: return w.title;
            case fields::bonus: return w.attackBonus(m_bonusMap);
            case fields::damage: return w.damage;
            case fields::notes: return w.notes;
        }
    }
    return {};
}

Qt::ItemFlags DndAttackModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    if (index.column() == fields::del)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

bool DndAttackModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || index.row() >= m_attackList.size())
        return false;

    Attack &w = m_attackList[index.row()];
    QString strVal = value.toString();

    switch (index.column()) {
        case fields::title: w.title = strVal; break;
        case fields::damage: w.damage = strVal; break;
        case fields::notes: w.notes = strVal; break;
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
    for (int i = 0; i < m_attackList.size(); ++i) {
        Attack attack = m_attackList[i];
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
                break;
            }
        }
    }
    return m_attackArray;
}

void DndAttackModel::editAttack(int row, const Attack& attack) {
    if (row < 0 || row >= m_attackList.size())
        return;
    m_attackList[row] = attack;
    emit dataChanged(index(row, 0), index(row, fields::del));
}


DndResourceModel::DndResourceModel(QObject *parent) : QAbstractTableModel(parent) {
    ThemedIconManager::instance().addPixmapTarget(":/charSheet/shortrest.svg", this, [=](const QPixmap& px){ m_shortRestIcon = QIcon(px);});
    ThemedIconManager::instance().addPixmapTarget(":/charSheet/longrest.svg", this, [=](const QPixmap& px){ m_longRestIcon = QIcon(px);});
}

QVariant DndResourceModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_resourcesList.size())
        return {};
    const Resource &r = m_resourcesList.at(index.row());

    if (role == Qt::DecorationRole && index.column() == fields::refill){
        if (r.refillOnShortRest) return m_shortRestIcon;
        if (r.refillOnLongRest) return m_longRestIcon;
        return {};
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::UserRole) {
        if (role == Qt::DisplayRole && index.column() == fields::del)
            return "❌";

        switch (index.column()) {
            case fields::title: return r.title;
            case fields::current: return r.current;
            case fields::max: return r.max;
        }
    }
    return {};
}

QVariant DndResourceModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole && role != Qt::DecorationRole || orientation != Qt::Horizontal)
        return {};

    switch (section) {
        case fields::refill: return tr("Refill");
        case fields::title: return tr("Title");
        case fields::current: return tr("Current");
        case fields::max: return tr("Max");
        case fields::del: return tr("Delete");
        default: return {};
    }
}

Qt::ItemFlags DndResourceModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    if (index.column() == fields::del || index.column() == fields::refill)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

bool DndResourceModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || index.row() >= m_resourcesList.size())
        return false;

    Resource &r = m_resourcesList[index.row()];
    QString strVal = value.toString();

    switch (index.column()) {
        case fields::title: r.title = strVal; break;
        case fields::current: r.current = strVal.toInt(); break;
        case fields::max: r.max = strVal.toInt(); break;
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
    emit dataChanged(index(0, fields::current), index(m_resourcesList.size()-1, fields::current+1));
}

void DndResourceModel::doShortRest() {
    for (auto& r: m_resourcesList) {
        if (r.refillOnShortRest) r.current = r.max;
    }
    emit dataChanged(index(0, fields::current), index(m_resourcesList.size()-1, fields::current+1));
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
    emit dataChanged(index(row, refill), index(row, refill+1));
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
