#include "initiativemodel.h"
#include <QBrush>
#include <QColor>
#include <QDomDocument>
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>
#include <QJSEngine> // Для вычисления арифметики

InitiativeModel::InitiativeModel(QObject *parent)
        : QAbstractTableModel(parent) {}

// Кол-во строк
int InitiativeModel::rowCount(const QModelIndex &) const {
    return characters.size();
}

// Кол-во колонок
int InitiativeModel::columnCount(const QModelIndex &) const {
    return 6; // name, initiative, ac, hp, maxHp, DELETE
}

// Заголовки
QVariant InitiativeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();

    switch (section) {
        case 0: return tr("Name");
        case 1: return tr("Initiative");
        case 2: return tr("AC");
        case 3: return tr("HP");
        case 4: return tr("Max HP");
        case 5: return tr("Delete");
        default: return QVariant();
    }
}

/**
 * @brief Возвращает данные для отображения в таблице.
 * @param index Индекс ячейки
 * @param role Роль данных (отображение, редактирование и т.д.)
 * @return QVariant с нужными данными
 */
QVariant InitiativeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= characters.size())
        return QVariant();

    const InitiativeCharacter &c = characters.at(index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (role == Qt::DisplayRole && index.column() == 5)
            return "❌";

        if (index.column() == 3) { // HP column
            const auto &c = characters.at(index.row());
            bool ok;
            int hpVal = evaluateExpression(c.hp, &ok);
            if (!ok) return "?";

            if (role == Qt::DisplayRole)
                return hpVal;
            if (role == Qt::UserRole)
                return c.maxHp;
        }

        switch (index.column()) {
            case 0: return c.name;
            case 1: return c.initiative;
            case 2: return c.ac;
            case 3: return c.hp;
            case 4: return c.maxHp;
        }
    }

    if (role == Qt::BackgroundRole && index.row() == currentIndex) {
        return QBrush(QColor("#cceeff")); // Подсветка текущего
    }

    return QVariant();
}

// Флаги редактирования
Qt::ItemFlags InitiativeModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    if (index.column() == 5)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

// Установка данных (и обработка выражений)
bool InitiativeModel::setData(const QModelIndex &index, const QVariant &value, int) {
    if (!index.isValid() || index.row() >= characters.size())
        return false;

    InitiativeCharacter &c = characters[index.row()];
    QString strVal = value.toString();

    switch (index.column()) {
        case 0: c.name = strVal; break;
        case 1: c.initiative = strVal.toInt(); break;
        case 2: c.ac = strVal.toInt(); break;
        case 3:
            c.hp = strVal;
            evaluateHP(index.row()); // вычисляем выражение
            break;
        case 4: c.maxHp = strVal.toInt(); break;
        default: return false;
    }

    emit dataChanged(index, index);
    return true;
}

// Добавление персонажа
void InitiativeModel::addCharacter(const InitiativeCharacter &character) {
    beginInsertRows(QModelIndex(), characters.size(), characters.size());
    characters.append(character);
    endInsertRows();
}

// Удаление персонажа
void InitiativeModel::removeCharacter(int row) {
    if (row < 0 || row >= characters.size()) return;

    beginRemoveRows(QModelIndex(), row, row);
    characters.removeAt(row);
    endRemoveRows();

    if (currentIndex >= row && currentIndex > 0)
        --currentIndex;
}

// Сортировка по инициативе
void InitiativeModel::sortByInitiative() {
    std::sort(characters.begin(), characters.end(), [](const InitiativeCharacter &a, const InitiativeCharacter &b) {
        return a.initiative > b.initiative;
    });
    emit dataChanged(index(0,0), index(rowCount()-1, columnCount()-1));
}

// Возврат персонажа
InitiativeCharacter InitiativeModel::getCharacter(int row) const {
    if (row >= 0 && row < characters.size())
        return characters[row];
    return {};
}

// Подсветка текущего активного
void InitiativeModel::setCurrentIndex(int index) {
    if (index < 0 || index >= characters.size()) return;

    int old = currentIndex;
    currentIndex = index;

    emit dataChanged(this->index(old, 0), this->index(old, columnCount()-1));
    emit dataChanged(this->index(currentIndex, 0), this->index(currentIndex, columnCount()-1));
}

int InitiativeModel::getCurrentIndex() const {
    return currentIndex;
}

// Вычисление выражения в HP
void InitiativeModel::evaluateHP(int row) {
    if (row < 0 || row >= characters.size()) return;

    InitiativeCharacter &c = characters[row];
    QJSEngine engine;
    QJSValue result = engine.evaluate(c.hp);
    if (result.isNumber()) {
        c.hp = QString::number(result.toInt());
        emit dataChanged(index(row, 3), index(row, 3)); // HP колонка
    }
}


bool InitiativeModel::saveToFile(const QString &filename) const {
    QDomDocument doc("InitiativeTracker");
    QDomElement root = doc.createElement("initiative");
    doc.appendChild(root);

    for (const InitiativeCharacter &c : characters) {
        QDomElement charElem = doc.createElement("character");
        charElem.setAttribute("name", c.name);
        charElem.setAttribute("initiative", c.initiative);
        charElem.setAttribute("ac", c.ac);
        charElem.setAttribute("hp", c.hp);
        charElem.setAttribute("maxhp", c.maxHp);
        root.appendChild(charElem);
    }

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    doc.save(out, 4);
    return true;
}

bool InitiativeModel::loadFromFile(const QString &filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }
    file.close();

    QDomElement root = doc.documentElement();
    if (root.tagName() != "initiative")
        return false;

    beginResetModel();
    characters.clear();

    QDomNodeList charNodes = root.elementsByTagName("character");
    for (int i = 0; i < charNodes.count(); ++i) {
        QDomElement elem = charNodes.at(i).toElement();
        InitiativeCharacter c;
        c.name = elem.attribute("name");
        c.initiative = elem.attribute("initiative").toInt();
        c.ac = elem.attribute("ac").toInt();
        c.hp = elem.attribute("hp");
        c.maxHp = elem.attribute("maxhp").toInt();
        characters.append(c);
    }
    endResetModel();
    return true;
}

static int evaluateExpression(const QString &expression, bool *ok) {
    QJSEngine engine;
    QJSValue result = engine.evaluate(expression);

    if (result.isError() || !result.isNumber()) {
        if (ok) *ok = false;
        return 0;
    }

    if (ok) *ok = true;
    return result.toInt();
}