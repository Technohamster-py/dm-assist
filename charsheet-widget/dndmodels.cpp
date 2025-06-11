#include "dndmodels.h"

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
    if (!index.isValid() || index.row() >= m_weaponList.size())
        return QVariant();

    const Weapon &w = m_weaponList.at(index.row());

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
    if (!index.isValid() || index.row() >= m_weaponList.size())
        return false;

    Weapon &w = m_weaponList[index.row()];
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

void DndAttackModel::addWeapon(const Weapon &weapon) {
    beginInsertRows(QModelIndex(), m_weaponList.size(), m_weaponList.size());
    m_weaponList.append(weapon);
    endInsertRows();
}

void DndAttackModel::deleteWeapon(int row) {
    if (row < 0 || row >= m_weaponList.size()) return;

    beginRemoveRows(QModelIndex(), row, row);
    m_weaponList.removeAt(row);
    endRemoveRows();
}

Weapon DndAttackModel::getWeapon(int row) const {
    if (row >= 0 && row < m_weaponList.size())
        return m_weaponList[row];
    return {};
}

bool DndAttackModel::fromJson(const QJsonArray& attackList) {
    for (const auto& attackVal: attackList) {
        QJsonObject attack = attackVal.toObject();
        Weapon weapon;

        weapon.title = attack["name"].toObject()["value"].toString();
        weapon.damage = attack["dmg"].toObject()["value"].toString();
        weapon.ability = attack["ability"].toString();
        weapon.prof = attack["isProf"].toBool();
        weapon.bonus = attack["modBonus"].toObject()["value"].toInt();
        weapon.notes = attack["notes"].toObject()["value"].toString();

        addWeapon(weapon);
    }
    return true;
}


