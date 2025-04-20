#include "initiativemodel.h"
#include <QBrush>
#include <QColor>
#include <QDomDocument>
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>
#include <QJSEngine> // Для вычисления арифметики

/**
 * @brief Конструктор модели инициативы.
 * @details Инициализирует базовую модель, используется в качестве источника данных для таблицы.
 * @param parent Родительский объект
 */
InitiativeModel::InitiativeModel(QObject *parent)
        : QAbstractTableModel(parent) {}

/**
 * @brief Возвращает количество строк в модели.
 * @details Количество строк соответствует количеству персонажей.
 * @param index Не используется
 * @return Количество строк
 */
int InitiativeModel::rowCount(const QModelIndex &) const {
    return characters.size();
}

/**
 * @brief Возвращает количество колонок в таблице.
 * @details Всегда возвращает 6: имя, инициатива, AC, HP, MaxHP и удаление.
 * @param parent Не используется
 * @return Количество колонок
 */
int InitiativeModel::columnCount(const QModelIndex &) const {
    return 6;
}

/**
 * @brief Возвращает заголовок колонки.
 * @details Используется для отображения заголовков в таблице.
 * @param section Индекс колонки
 * @param orientation Ориентация (только горизонтальная)
 * @param role Роль (только DisplayRole)
 * @return Название колонки или QVariant()
 */
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
 * @brief Возвращает данные ячейки.
 * @details Обрабатывает отображение, редактирование, фоновую подсветку и отображение выражений в HP.
 * @param index Индекс ячейки
 * @param role Роль (DisplayRole, EditRole, BackgroundRole, UserRole)
 * @return QVariant с содержимым ячейки
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

/**
 * @brief Возвращает флаги редактируемости ячейки.
 * @details Разрешает редактирование всех ячеек кроме колонки Delete.
 * @param index Индекс ячейки
 * @return Флаги Qt::ItemFlags
 */
Qt::ItemFlags InitiativeModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    if (index.column() == 5)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

/**
 * @brief Устанавливает данные в модель.
 * @details Обновляет содержимое модели и вызывает сигнал dataChanged.
 * @param index Индекс редактируемой ячейки
 * @param value Новое значение
 * @param role Не используется (по умолчанию)
 * @return true если установка прошла успешно
 */
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

/**
 * @brief Добавляет персонажа в модель.
 * @details Вставляет новую строку в конец модели и обновляет представление.
 * @param character Новый персонаж
 */
void InitiativeModel::addCharacter(const InitiativeCharacter &character) {
    beginInsertRows(QModelIndex(), characters.size(), characters.size());
    characters.append(character);
    endInsertRows();
}

/**
 * @brief Удаляет персонажа из модели.
 * @details Удаляет строку с указанным индексом, корректирует текущий индекс.
 * @param row Индекс строки для удаления
 */
void InitiativeModel::removeCharacter(int row) {
    if (row < 0 || row >= characters.size()) return;

    beginRemoveRows(QModelIndex(), row, row);
    characters.removeAt(row);
    endRemoveRows();

    if (currentIndex >= row && currentIndex > 0)
        --currentIndex;
}

/**
 * @brief Сортирует модель по инициативе.
 * @details Сортирует персонажей по убыванию инициативы и обновляет представление.
 */
void InitiativeModel::sortByInitiative() {
    std::sort(characters.begin(), characters.end(), [](const InitiativeCharacter &a, const InitiativeCharacter &b) {
        return a.initiative > b.initiative;
    });
    emit dataChanged(index(0,0), index(rowCount()-1, columnCount()-1));
}

/**
 * @brief Возвращает объект персонажа по строковому индексу.
 * @details Используется для получения полной информации о персонаже.
 * @param row Индекс строки
 * @return Объект InitiativeCharacter или пустой, если индекс некорректен
 */
InitiativeCharacter InitiativeModel::getCharacter(int row) const {
    if (row >= 0 && row < characters.size())
        return characters[row];
    return {};
}

/**
 * @brief Устанавливает текущего активного персонажа.
 * @details Обновляет индекс текущей строки и вызывает обновление отображения.
 * @param index Индекс текущего персонажа
 */
void InitiativeModel::setCurrentIndex(int index) {
    if (index < 0 || index >= characters.size()) return;

    int old = currentIndex;
    currentIndex = index;

    emit dataChanged(this->index(old, 0), this->index(old, columnCount()-1));
    emit dataChanged(this->index(currentIndex, 0), this->index(currentIndex, columnCount()-1));
}

/**
 * @brief Получает индекс текущего персонажа.
 * @details Возвращает индекс строки, которая считается активной.
 * @return Индекс текущей строки
 */
int InitiativeModel::getCurrentIndex() const {
    return currentIndex;
}

/**
 * @brief Вычисляет арифметическое выражение в HP.
 * @details Использует движок JavaScript для вычисления выражений типа "100 - 25".
 * @param row Индекс строки, в которой обновляется HP
 */
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

/**
 * @brief Сохраняет модель в XML-файл.
 * @details Сохраняет всех персонажей и их данные в структуру XML.
 * @param filename Имя файла
 * @return true, если сохранение прошло успешно
 */
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

/**
 * @brief Загружает модель из XML-файла.
 * @details Считывает персонажей из структуры XML и заполняет модель.
 * @param filename Имя файла
 * @return true, если загрузка прошла успешно
 */
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

/**
 * @brief Вычисляет строковое арифметическое выражение.
 * @details Используется для парсинга HP, например "100-25" -> 75.
 * @param expression Строка выражения
 * @param ok Указатель для флага успеха
 * @return Результат вычисления или 0, если ошибка
 */
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