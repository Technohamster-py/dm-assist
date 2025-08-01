#include "initiativemodel.h"
#include <QBrush>
#include <QApplication>
#include <QDomDocument>
#include <QFile>
#include <QPalette>
#include <QRegularExpression>
#include <QTextStream>
#include <QJSEngine>
#include <themediconmanager.h>


InitiativeModel::InitiativeModel(QObject *parent)
        : QAbstractTableModel(parent) {
    m_characterHeaderIcon = QIcon(":/human.svg");
    m_initHeaderIcon = QIcon(":/d20.svg");
    m_acHeaderIcon = QIcon(":/shield.svg");
    m_hpHeaderIcon = QIcon(":/heart-broken.svg");
    m_hpHeaderIcon = QIcon(":/heart.svg");
    m_speedHeaderIcon = QIcon(":/charSheet/run.svg");

    ThemedIconManager::instance().addPixmapTarget(":/human.svg", this,
                                                  [=](const QPixmap &px) { m_characterHeaderIcon = QIcon(px); }, false);
    ThemedIconManager::instance().addPixmapTarget(":/d20.svg", this,
                                                  [=](const QPixmap &px) { m_initHeaderIcon = QIcon(px); }, false);
    ThemedIconManager::instance().addPixmapTarget(":/shield.svg", this,
                                                  [=](const QPixmap &px) { m_acHeaderIcon = QIcon(px); }, false);
    ThemedIconManager::instance().addPixmapTarget(":/heart-broken.svg", this,
                                                  [=](const QPixmap &px) { m_hpHeaderIcon = QIcon(px); }, false);
    ThemedIconManager::instance().addPixmapTarget(":/heart.svg", this,
                                                  [=](const QPixmap &px) { m_hpMaxHeaderIcon = QIcon(px); }, false);
    ThemedIconManager::instance().addPixmapTarget(":/charSheet/run.svg", this,
                                                  [=](const QPixmap &px) { m_speedHeaderIcon = QIcon(px); }, false);

    connect(&ThemedIconManager::instance(), &ThemedIconManager::themeChanged, [=](){
        emit dataChanged(index(0, fields::statuses), index(rowCount()-1, fields::statuses), {Qt::DecorationRole});
    });
}

/**
 * @brief Returns the number of rows in the model.
 *
 * This function calculates and returns the total number of rows in the model
 * by retrieving the size of the `characters` container, which stores the
 * initiative characters.
 *
 * @param parent Not used in this implementation. It exists to adhere to the
 *               QAbstractItemModel interface.
 * @return The number of rows, which corresponds to the number of elements in
 *         the `characters` vector.
 */
int InitiativeModel::rowCount(const QModelIndex &) const {
    return characters.size();
}


/**
 * @brief Returns the number of columns in the model.
 *
 * This function provides the total number of columns available in the model, which is fixed at 6.
 * Each column corresponds to a specific field as defined in the model such as name, initiative,
 * AC, HP, maximum HP, and delete action.
 *
 * @param unused This parameter is required to match the method signature but is unused in the implementation.
 * @return int The total number of columns in the model, always 6.
 */
int InitiativeModel::columnCount(const QModelIndex &) const {
    return fields::del+1;
}


/**
 * @brief Retrieves header data for the model.
 *
 * This function provides data for the horizontal headers of the model, displaying
 * specific text or icons depending on the column section and role provided.
 * It processes requests for `Qt::DisplayRole` and `Qt::DecorationRole` only.
 *
 * @param section The section index of the header (column index).
 * @param orientation The orientation of the header. Only `Qt::Horizontal` is handled.
 * @param role The role of the data requested. Typically `Qt::DisplayRole` or `Qt::DecorationRole`.
 * @return A QVariant containing the header data. Returns header text for specific sections
 *         (e.g., Name, Max, Delete) or an icon (e.g., for initiative, armor class, and hit points),
 *         or an invalid QVariant if the parameters do not match the handled cases.
 */
QVariant InitiativeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole && role != Qt::DecorationRole || orientation != Qt::Horizontal)
        return {};

    switch (section) {
        case fields::name: return m_characterHeaderIcon;
        case fields::initiative: return m_initHeaderIcon;
        case fields::speed: return m_speedHeaderIcon;
        case fields::statuses: return "statuses";
        case fields::Ac: return m_acHeaderIcon;
        case fields::hp: return m_hpHeaderIcon;
        case fields::maxHp: return m_hpMaxHeaderIcon;
        default: return {};
    }
}


/**
 * @brief Retrieves data for a given index and role in the model.
 * @details Depending on the role and column, provides data for display, editing, or other purposes.
 *          Supports both standard display roles (e.g., Qt::DisplayRole, Qt::EditRole) and custom roles.
 * @param index The model index for which data is requested.
 * @param role The role that specifies the purpose of the requested data (e.g., Qt::DisplayRole, Qt::EditRole, Qt::BackgroundRole, etc.).
 * @return The data corresponding to the given index and role. Returns an invalid QVariant if the index is invalid or out of bounds,
 *         or if the role is not supported.
 *
 * - For `Qt::DisplayRole` or `Qt::EditRole`:
 *   - Column `0`: Returns the character's name.
 *   - Column `1`: Returns the character's initiative.
 *   - Column `2`: Returns the character's AC (armor class).
 *   - Column `3` (HP column): Evaluates the expression if possible and returns the HP value.
 *     - If `Qt::UserRole` is used, it returns the character's maximum HP.
 *   - Column `4`: Returns the maximum HP value.
 *   - Column `5`: Returns the delete symbol ("❌").
 * - For `Qt::BackgroundRole`:
 *   - Highlights the row for the current index with a specific background color.
 *
 * If a role is not handled or the index/row is invalid, returns an invalid QVariant.
 */
QVariant InitiativeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= characters.size())
        return {};

    const InitiativeCharacter &c = characters.at(index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::UserRole) {
        if (role == Qt::DisplayRole && index.column() == fields::del)
            return "❌";

        if (index.column() == fields::hp) { // HP column
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
            case fields::name: return c.name;
            case fields::initiative: return c.initiative;
            case fields::speed: return c.speed;
            case fields::Ac: return c.ac;
            case fields::hp: return c.hp;
            case fields::maxHp: return c.maxHp;
        }
    }

    if (role == Qt::BackgroundRole && index.row() == currentIndex) {
        return QBrush(QApplication::palette().color(QPalette::Highlight)); // Подсветка текущего
    }

    if (role == Qt::DecorationRole && index.column() == fields::statuses){
       QStringList iconPaths;
        for (const auto status : characters[index.row()].statuses) {
            iconPaths << status.iconPath;
        }
        return ThemedIconManager::instance().renderIconGrid(iconPaths, QSize(m_iconHeight, m_iconHeight), m_iconSpacing, m_iconsPerRow);
    }

    if (role == Qt::SizeHintRole && index.column() == fields::statuses){
        const int count = characters[index.row()].statuses.size();

        int rows = (count + m_iconsPerRow - 1) / m_iconsPerRow;
        int height = rows * iconHeight() + (rows - 1) * m_iconSpacing;

        return QSize(-1, height);
    }

    if (role == Qt::UserRole + 1 && index.column() == fields::statuses)
        return QVariant::fromValue(characters[index.row()].statuses);

    return {};
}


/**
 * Determines the item flags for a specific index in the model.
 *
 * @param index The QModelIndex representing the specific item in the model.
 *
 * @return Qt::ItemFlags containing the flags applicable to the item at the specified index.
 *         - If the index is invalid, returns Qt::NoItemFlags.
 *         - If the column of the index is 5 (corresponding to the "del" field),
 *           allows the item to only be selectable and enabled (Qt::ItemIsSelectable | Qt::ItemIsEnabled).
 *         - For all other columns, the item is selectable, enabled, and editable
 *           (Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable).
 */
Qt::ItemFlags InitiativeModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    if (index.column() == fields::del || index.column() == fields::statuses)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}


/**
 * @brief Updates the data of the model at a specified index.
 *
 * @param index The QModelIndex indicating the row and column of the data to be updated.
 * @param value The new value for the specified cell, encapsulated in a QVariant.
 * @param NotUsed This parameter is unused.
 * @return Returns true if the data was successfully updated; otherwise, returns false.
 *
 * @details This function updates the value of a specific cell in the model based on its row
 * and column. It modifies the corresponding field of an `InitiativeCharacter` object within
 * the `characters` vector. The behavior depends on the column:
 *
 * - Column 0: Updates the character's name.
 * - Column 1: Updates the character's initiative value.
 * - Column 2: Updates the character's armor class (AC).
 * - Column 3: Updates the character's hit points (HP). If the HP contains an arithmetic
 *   expression (e.g., "50-20"), it will be evaluated using the `evaluateHP` function.
 * - Column 4: Updates the character's maximum HP.
 *
 * If the index is invalid or out of range, or if the column is not handled, the function
 * returns false without making changes. If the update is successful, the `dataChanged`
 * signal is emitted for the specified index to notify views of the change.
 */
bool InitiativeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || index.row() >= characters.size())
        return false;

    InitiativeCharacter &c = characters[index.row()];
    QString strVal = value.toString();

    switch (index.column()) {
        case fields::name: c.name = strVal; break;
        case fields::initiative: c.initiative = strVal.toInt(); break;
        case fields::speed: c.speed = strVal.toInt(); break;
        case fields::Ac: c.ac = strVal.toInt(); break;
        case fields::hp:
            c.hp = strVal;
            evaluateHP(index.row()); // вычисляем выражение
            break;
        case fields::maxHp: c.maxHp = strVal.toInt(); break;
        case fields::statuses:
            if (value.canConvert<QList<Status>>())
                c.statuses = value.value<QList<Status>>();
            emit dataChangedExternally();
            break;
        default: return false;
    }

    emit dataChanged(index, index);
    return true;
}


/**
 * @brief Adds a new InitiativeCharacter to the model.
 *
 * This function inserts a new character into the internal storage of the model
 * and notifies any associated views of the change by emitting the appropriate
 * signals. The character is appended to the end of the character list.
 *
 * @param character The InitiativeCharacter object to be added to the model.
 */
void InitiativeModel::addCharacter(const InitiativeCharacter &character) {
    beginInsertRows(QModelIndex(), characters.size(), characters.size());
    characters.append(character);
    endInsertRows();
}


/**
 * @brief Removes a character from the model at the specified row index.
 *
 * This function removes a character from the internal list of characters
 * (the `characters` vector) stored in the model at the specified row index.
 * It updates the data model accordingly and ensures model consistency.
 *
 * If the specified row index is invalid (out of bounds), the function does nothing.
 * If the current index (`currentIndex`) is affected by the removal, it is updated
 * to maintain consistency.
 *
 * The function emits the necessary signals (`beginRemoveRows` and `endRemoveRows`)
 * to notify views about the changes in the data.
 *
 * @param row The row index of the character to be removed. It must be a valid
 *            index within the bounds of the `characters` vector.
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
 * @brief Sorts the characters by their initiative values in descending order.
 *
 * This function rearranges the `characters` vector so that the elements
 * are ordered based on their `initiative` attribute in descending order.
 * After sorting, it emits the `dataChanged` signal to notify any connected
 * views that the model's data has been updated.
 *
 * The sorting is performed using the `std::sort` algorithm with a lambda
 * function comparator that compares the `initiative` field of two
 * `InitiativeCharacter` objects.
 */
void InitiativeModel::sortByInitiative() {
    std::sort(characters.begin(), characters.end(), [](const InitiativeCharacter &a, const InitiativeCharacter &b) {
        return a.initiative > b.initiative;
    });
    emit dataChanged(index(0,0), index(rowCount()-1, columnCount()-1));
}


/**
 * Retrieves an InitiativeCharacter object from the specified row in the characters list.
 *
 * This function returns a copy of the InitiativeCharacter stored at the given row index.
 * If the row index is invalid (less than 0 or greater than or equal to the size of the characters list),
 * a default-constructed InitiativeCharacter is returned.
 *
 * @param row The zero-based index of the row to retrieve the character from.
 * @return An InitiativeCharacter object representing the character at the specified row,
 *         or a default-constructed InitiativeCharacter if the row index is invalid.
 */
InitiativeCharacter InitiativeModel::getCharacter(int row) const {
    if (row >= 0 && row < characters.size())
        return characters[row];
    return {};
}


/**
 * @brief Updates the current index of the selected character in the model.
 *
 * This function changes the `currentIndex` to the specified `index` if it falls within the valid range.
 * It ensures that the index is greater than or equal to 0 and less than the size of the `characters` vector.
 * After updating the current index, it emits the `dataChanged` signal to notify any views that the data
 * for the old and new current index rows has changed, causing them to refresh.
 *
 * @param index The new current index to be set. Must be within the range
 *              [0, characters.size()) to be valid; otherwise, no change is made.
 */
void InitiativeModel::setCurrentIndex(int index) {
    if (index < 0 || index >= characters.size()) return;

    int old = currentIndex;
    currentIndex = index;

    emit dataChanged(this->index(old, 0), this->index(old, columnCount()-1));
    emit dataChanged(this->index(currentIndex, 0), this->index(currentIndex, columnCount()-1));
}


/**
 * @brief Retrieves the current index of the model.
 *
 * This function returns the value of the `currentIndex` member, which represents
 * the currently selected or active index within the initiative model.
 *
 * @return The current index as an integer.
 */
int InitiativeModel::getCurrentIndex() const {
    return currentIndex;
}


/**
 * @brief Evaluates and updates the hit points (HP) of a character at a specified row.
 *
 * This function interprets the HP value of the character at the specified row as a
 * mathematical expression. The expression is evaluated using QJSEngine, and if the
 * result is a valid number, the character's HP is updated to the calculated value.
 * The UI is notified of the update in the HP column.
 *
 * @param row The index of the character in the model for which the HP should be evaluated.
 *            If the row index is invalid (negative or out of range), the function does nothing.
 *
 * Emits:
 * - dataChanged: This signal is emitted to notify views that the HP value in the specified row
 *                and column has been updated.
 */
void InitiativeModel::evaluateHP(int row) {
    if (row < 0 || row >= characters.size()) return;

    InitiativeCharacter &c = characters[row];
    QJSEngine engine;
    QJSValue result = engine.evaluate(c.hp);
    if (result.isNumber()) {
        c.hp = QString::number(result.toInt());
        emit dataChanged(index(row, fields::hp), index(row, fields::hp)); // HP колонка
    }
}


/**
 * @brief Saves the InitiativeModel data to an XML file.
 *
 * The method serializes the list of InitiativeCharacter objects
 * into an XML format and writes it to the specified file.
 * Each character is represented as an XML element with relevant
 * attributes like name, initiative, armor class (ac), current hit points (hp),
 * and maximum hit points (maxHp). The root element of the XML structure
 * is named 'initiative', and child elements are labeled 'character'.
 *
 * @param filename The name of the file to which the data is saved.
 * @return Returns true if the file was successfully written, false otherwise.
 *
 * @details If the file cannot be opened for writing, the method returns false.
 * Otherwise, it writes the serialized XML document to the file and returns true.
 */
bool InitiativeModel::saveToFile(const QString &filename) const {
    if (filename.isEmpty())
        return false;
    QDomDocument doc("InitiativeTracker");
    QDomElement root = doc.createElement("initiative");
    doc.appendChild(root);

    for (const InitiativeCharacter &c : characters) {
        QDomElement charElem = doc.createElement("character");
        charElem.setAttribute("name", c.name);
        charElem.setAttribute("initiative", c.initiative);
        charElem.setAttribute("speed", c.speed);
        charElem.setAttribute("ac", c.ac);
        charElem.setAttribute("hp", c.hp);
        charElem.setAttribute("maxhp", c.maxHp);

        for (auto status : c.statuses) {
            QDomElement statElem = doc.createElement("status");
            statElem.setAttribute("title", status.title);
            statElem.setAttribute("icon", status.iconPath);
            statElem.setAttribute("remaining", status.remainingRounds);
            charElem.appendChild(statElem);
        }

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
 * @brief Loads initiative data from a file and updates the model.
 *
 * This function resets the model, clears the current list of characters,
 * reads character data from the specified file using `addFromFile()`,
 * and then resets the model to reflect the new data. It ensures that the
 * model is properly updated upon loading data.
 *
 * @param filename The path to the file containing initiative data.
 * @return Returns true if the file data is processed and the model is updated.
 */
bool InitiativeModel::loadFromFile(const QString &filename) {
    beginResetModel();
    characters.clear();
    addFromFile(filename);
    endResetModel();
    return true;
}

/**
 * @brief Parses and adds characters from an XML file to the model.
 *
 * This function reads an XML file specified by the filename parameter, parses it, and appends the characters
 * defined in the XML to the internal character list of the InitiativeModel. The expected XML format should contain
 * a root element named "initiative", within which each character is represented as a "character" element with
 * attributes "name", "initiative", "ac", "hp", and "maxhp".
 *
 * @param filename The path to the XML file from which characters will be loaded.
 * @return true if the file was successfully parsed and characters were added; otherwise, false.
 *
 * The function performs the following steps:
 * - Opens the file in a read-only, text mode. Returns false if file opening fails.
 * - Parses the content of the file as an XML document. Returns false if parsing fails.
 * - Checks whether the root element of the XML document is "initiative". Returns false otherwise.
 * - Iterates over all "character" elements within the root "initiative" element, extracting their attributes
 *   to populate the corresponding fields of InitiativeCharacter objects.
 * - Adds the created InitiativeCharacter objects to the internal character list.
 * - Resets the model's internal data to reflect the new additions.
 */
bool InitiativeModel::addFromFile(const QString &filename) {
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
    QDomNodeList charNodes = root.elementsByTagName("character");
    for (int i = 0; i < charNodes.count(); ++i) {
        QDomElement elem = charNodes.at(i).toElement();
        InitiativeCharacter c;
        c.name = elem.attribute("name");
        c.initiative = elem.attribute("initiative").toInt();
        c.speed = elem.attribute("speed").toInt();
        c.ac = elem.attribute("ac").toInt();
        c.hp = elem.attribute("hp");
        c.maxHp = elem.attribute("maxhp").toInt();

        QDomNodeList statNodes = elem.elementsByTagName("status");
        for (int j = 0; j < statNodes.count(); ++j) {
            QDomElement statElem = statNodes.at(j).toElement();
            Status status;
            status.title = statElem.attribute("title");
            status.iconPath = statElem.attribute("icon");
            status.remainingRounds = statElem.attribute("remaining").toInt();

            c.statuses.append(status);
        }

        characters.append(c);
    }
    endResetModel();
    return true;
}

void InitiativeModel::decrementStatuses() {
    for (auto &character : characters) {
        auto it = character.statuses.begin();
        while (it != character.statuses.end()){
            it->remainingRounds --;
            if (it->remainingRounds <= 0)
                it = character.statuses.erase(it);
            else
                ++it;
        }
    }
    emit dataChanged(index(0, fields::statuses), index(rowCount()-1, fields::statuses));
}


/**
 * @brief Evaluates a mathematical expression represented as a QString.
 *
 * This function takes a mathematical expression as input in QString format
 * and evaluates it using QJSEngine. The result is checked for errors and
 * validated to be a numeric result. If the evaluation fails, the function
 * sets the ok pointer to false and returns 0. If the evaluation is successful,
 * it sets the ok pointer to true and returns the integer value of the result.
 *
 * @param expression The mathematical expression to evaluate as a QString.
 * @param ok A pointer to a boolean used to indicate the success of the operation.
 *           If the pointer is not nullptr, it will be set to true if the evaluation
 *           is successful, and false otherwise.
 * @return The integer value of the evaluated expression if successful, or 0 if the
 *         evaluation fails.
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