#include "dndcharsheetwidget.h"
#include "ui_dndcharsheetwidget.h"

#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextList>
#include "dndcharsheetdialogs.h"
#include <themediconmanager.h>


DndCharsheetWidget::DndCharsheetWidget(QWidget* parent) :
        AbstractCharsheetWidget(parent), ui(new Ui::DndCharsheetWidget) {
    ui->setupUi(this);

    attackModel = new DndAttackModel(this);
    ui->attacsView->setModel(attackModel);
    ui->attacsView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->attacsView->setDropIndicatorShown(true);


    resourceModel = new DndResourceModel(this);
    ui->resourcesView->setModel(resourceModel);
    ui->resourcesView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->resourcesView->setDropIndicatorShown(true);

    setupShortcuts();
    connectSignals();

    connect(ui->attacsView, &QTableView::clicked, this, [=](const QModelIndex &index) {
        switch (index.column()) {
            case DndAttackModel::fields::del:
                attackModel->deleteAttack(index.row());
                break;
            case DndAttackModel::fields::damage:
                emit rollRequested(attackModel->getAttack(index.row()).damage);
                break;
            default:
                int bonus = attackModel->getAttack(index.row()).bonus;
                QString strBonus = (bonus >= 0) ? "+" + QString::number(bonus) : QString::number(bonus);
                emit rollRequested(QString("d20 %1").arg(strBonus));
                break;
        }
    });

    connect(ui->attacsView, &QTableView::doubleClicked, [=](const QModelIndex &index){
        if (index.column() == DndAttackModel::fields::del)
            return;

        AttackDialog attackDialog(this, attackModel->getAttack(index.row()));
        if (attackDialog.exec()== QDialog::Accepted){
            Attack attack = attackDialog.getCreatedAttack();
            attackModel->editAttack(index.row(), attack);
        }
    });

    connect(ui->resourcesView, &QTableView::clicked, this, [=](const QModelIndex &index) {
        switch (index.column()) {
            case DndResourceModel::fields::del:
                resourceModel->deleteResource(index.row());
                break;
            case DndResourceModel::fields::refill:
                resourceModel->changeRefillMode(index.row());
                break;
            default:
                break;
        }
    });

    ui->resourcesView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->resourcesView->horizontalHeader()->setSectionResizeMode(DndResourceModel::fields::title, QHeaderView::Stretch);

    ui->attacsView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->attacsView->horizontalHeader()->setSectionResizeMode(DndAttackModel::fields::title, QHeaderView::Stretch);

    ThemedIconManager::instance().addIconTarget<QPushButton>(":/add.svg", ui->addResourceButton, &QAbstractButton::setIcon);
    ThemedIconManager::instance().addIconTarget<QPushButton>(":/add.svg", ui->addAttackButton, &QAbstractButton::setIcon);
    ThemedIconManager::instance().addIconTarget<QPushButton>(":/charSheet/longrest.svg", ui->longRestButton, &QAbstractButton::setIcon);
    ThemedIconManager::instance().addIconTarget<QPushButton>(":/charSheet/shortrest.svg", ui->shortRestButton, &QAbstractButton::setIcon);

    ThemedIconManager::instance().addPixmapTarget(":/shield.svg", ui->shieldIcon, [label = ui->shieldIcon](const QPixmap& px){label->setPixmap(px);});
    ThemedIconManager::instance().addPixmapTarget(":/charSheet/run.svg", ui->runIcon, [label = ui->runIcon](const QPixmap& px){label->setPixmap(px);});
}

DndCharsheetWidget::DndCharsheetWidget(const QString& filePath, QWidget *parent): DndCharsheetWidget(parent){
    loadFromFile(filePath);
}

DndCharsheetWidget::~DndCharsheetWidget() {
    delete ui;
}

/**
 * @brief Loads a character sheet from a JSON file.
 *
 * This method reads the specified file, parses its content as JSON,
 * and initializes the character widget with the data. If the file
 * cannot be opened, a warning message is displayed.
 *
 * @param path A QString representing the path to the character file.
 *
 * The method performs the following:
 * - Sets the internal `m_originalFilePath` to the provided file path.
 * - Attempts to open the specified file for reading. Displays an error
 *   dialog if the file cannot be opened.
 * - Reads the file content as JSON and stores it as a QJsonDocument in
 *   `m_originalDocument`.
 * - Extracts the "data" field from the JSON object, which is stored as
 *   a QJsonObject in `m_dataObject`.
 * - Calls `populateWidget()` to update the widget with the extracted data.
 */
void DndCharsheetWidget::loadFromFile(const QString &path) {
    m_originalFilePath = path;

    m_campaignPath = campaignDirFromFile(path);

    QFile characterFile(path);
    if (!characterFile.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this, "error", "Can't open character file");
        return;
    }
    m_originalDocument = QJsonDocument::fromJson(characterFile.readAll());
    QJsonObject rootObj = m_originalDocument.object();

    QString dataString = rootObj.value("data").toString();
    m_dataObject = QJsonDocument::fromJson(dataString.toUtf8()).object();

    characterFile.close();

    populateWidget();
}

/**
 * Populates the user interface of the character sheet widget with data extracted from m_dataObject.
 *
 * This method updates various UI components to reflect the data represented in m_dataObject,
 * including character information, abilities, proficiencies, skills, and other parameters.
 *
 * The data is parsed from multiple sections of the JSON object:
 * - Character details (e.g., name, class, level, proficiency bonus) are retrieved from the "info" field.
 * - Vitality attributes (e.g., speed, armor class, current and maximum hit points) are extracted from the "vitality" field.
 * - Ability scores and saving throw proficiencies are retrieved from the "stats" and "saves" fields.
 * - Skill proficiency states are extracted from the "skills" field.
 *
 * The method also calculates derived values such as ability score bonuses and proficiency bonuses
 * from the provided raw ability scores and level.
 *
 * Internal Functions Used:
 * - bonusFromStat(): Computes derived bonuses based on raw ability scores.
 * - proficiencyByLevel(): Calculates the proficiency bonus for a given level.
 *
 * UI Components Updated:
 * - Character Details:
 *   - Name, class, subclass, level, and proficiency bonus labels.
 * - Vitality:
 *   - Speed, armor class, current hit points, and maximum hit points fields.
 * - Abilities:
 *   - Strength, Dexterity, Constitution, Intelligence, Wisdom, and Charisma values and bonuses.
 *   - Associated saving throw proficiency checkboxes for each ability.
 * - Skills:
 *   - Updates the proficiency status of various skills such as athletics, stealth, arcana, and other character skills.
 *
 * Preconditions:
 * - m_dataObject must be a valid and properly structured QJsonObject.
 * - The UI components in the widget (e.g., nameLabel, levelBox, etc.) are initialized and accessible.
 * - The internal JSON structure must adhere to the expected schema for parsing (keys such as "name", "info",
 *   "vitality", "stats", "saves", and "skills" must be present and correctly formatted).
 *
 * This ensures that the widget displays up-to-date character data and can be used for dynamic visualization
 * or modification of character details within the application.
 */
void DndCharsheetWidget::populateWidget() {
    ui->nameEdit->setText(m_dataObject["name"].toObject()["value"].toString());


    QJsonObject infoObject = m_dataObject["info"].toObject();
    QString classString = QString("%1 (%2)").arg(infoObject["charClass"].toObject()["value"].toString(), infoObject["charSubclass"].toObject()["value"].toString());
    ui->classEdit->setText(classString);
    ui->levelBox->setValue(infoObject["level"].toObject()["value"].toInt());
    ui->proficiencyLabel->setText(QString::number(proficiencyByLevel(ui->levelBox->value())));


    QJsonObject vitalityObject = m_dataObject["vitality"].toObject();
    ui->speedBox->setValue(vitalityObject["speed"].toObject()["value"].toString().toInt());
    ui->acBox->setValue(vitalityObject["ac"].toObject()["value"].toInt());
    ui->hpSpinBox->setValue(vitalityObject["hp-current"].toObject()["value"].toInt());
    ui->maxHpBox->setValue(vitalityObject["hp-max"].toObject()["value"].toInt());


    QJsonObject statsObject = m_dataObject["stats"].toObject();
    QJsonObject savesObject = m_dataObject["saves"].toObject();
    ui->strValueEdit->setValue(statsObject["str"].toObject()["score"].toInt());
    ui->strBonusLabel->setText(QString::number(bonusFromStat(ui->strValueEdit->value())));
    ui->strSaveCheckBox->setChecked(true);
    ui->strSaveCheckBox->setChecked(savesObject["str"].toObject()["isProf"].toBool());

    ui->dexValueEdit->setValue(statsObject["dex"].toObject()["score"].toInt());
    ui->dexBonusLabel->setText(QString::number(bonusFromStat(ui->dexValueEdit->value())));
    ui->dexSaveCheckBox->setChecked(true);
    ui->dexSaveCheckBox->setChecked(savesObject["dex"].toObject()["isProf"].toBool());

    ui->conValueEdit->setValue(statsObject["con"].toObject()["score"].toInt());
    ui->conBonusLabel->setText(QString::number(bonusFromStat(ui->conValueEdit->value())));
    ui->conSaveCheckBox->setChecked(true);
    ui->conSaveCheckBox->setChecked(savesObject["con"].toObject()["isProf"].toBool());

    ui->intValueEdit->setValue(statsObject["int"].toObject()["score"].toInt());
    ui->intBonusLabel->setText(QString::number(bonusFromStat(ui->intValueEdit->value())));
    ui->intSaveCheckBox->setChecked(true);
    ui->intSaveCheckBox->setChecked(savesObject["int"].toObject()["isProf"].toBool());

    ui->wisValueEdit->setValue(statsObject["wis"].toObject()["score"].toInt());
    ui->wisBonusLabel->setText(QString::number(bonusFromStat(ui->wisValueEdit->value())));
    ui->wisSaveCheckBox->setChecked(true);
    ui->wisSaveCheckBox->setChecked(savesObject["wis"].toObject()["isProf"].toBool());

    ui->chaValueEdit->setValue(statsObject["cha"].toObject()["score"].toInt());
    ui->chaBonusLabel->setText(QString::number(bonusFromStat(ui->chaValueEdit->value())));
    ui->chaSaveCheckBox->setChecked(true);
    ui->chaSaveCheckBox->setChecked(savesObject["cha"].toObject()["isProf"].toBool());


    QJsonObject skillsObject = m_dataObject["skills"].toObject();
    ui->athlecicsCheckBox->setChecked(true);
    ui->athlecicsCheckBox->setChecked(skillsObject["athletics"].toObject()["isProf"].toInt());

    ui->acrobatics->setChecked(true);
    ui->acrobatics->setChecked(skillsObject["acrobatics"].toObject()["isProf"].toInt());

    ui->sleight->setChecked(true);
    ui->sleight->setChecked(skillsObject["sleight of hand"].toObject()["isProf"].toInt());

    ui->stealth->setChecked(true);
    ui->stealth->setChecked(skillsObject["stealth"].toObject()["isProf"].toInt());

    ui->arcana->setChecked(true);
    ui->arcana->setChecked(skillsObject["arcana"].toObject()["isProf"].toInt());

    ui->history->setChecked(true);
    ui->history->setChecked(skillsObject["history"].toObject()["isProf"].toInt());

    ui->investigation->setChecked(true);
    ui->investigation->setChecked(skillsObject["investigation"].toObject()["isProf"].toInt());

    ui->nature->setChecked(true);
    ui->nature->setChecked(skillsObject["nature"].toObject()["isProf"].toInt());

    ui->religion->setChecked(true);
    ui->religion->setChecked(skillsObject["religion"].toObject()["isProf"].toInt());

    ui->handling->setChecked(true);
    ui->handling->setChecked(skillsObject["animal handling"].toObject()["isProf"].toInt());

    ui->insight->setChecked(true);
    ui->insight->setChecked(skillsObject["insight"].toObject()["isProf"].toInt());

    ui->medicine->setChecked(true);
    ui->medicine->setChecked(skillsObject["medicine"].toObject()["isProf"].toInt());

    ui->perception->setChecked(true);
    ui->perception->setChecked(skillsObject["perception"].toObject()["isProf"].toInt());

    ui->survival->setChecked(true);
    ui->survival->setChecked(skillsObject["survival"].toObject()["isProf"].toInt());

    ui->deception->setChecked(true);
    ui->deception->setChecked(skillsObject["deception"].toObject()["isProf"].toInt());

    ui->intimidation->setChecked(true);
    ui->intimidation->setChecked(skillsObject["intimidation"].toObject()["isProf"].toInt());

    ui->performance->setChecked(true);
    ui->performance->setChecked(skillsObject["performance"].toObject()["isProf"].toInt());

    ui->persuasion->setChecked(true);
    ui->persuasion->setChecked(skillsObject["persuasion"].toObject()["isProf"].toInt());



    ui->proficienciesEdit->setHtml(parseParagraphs(m_dataObject["text"].toObject()["prof"].toObject()["value"].toObject()["data"].toObject()["content"].toArray()));
    ui->traitsEdit->setHtml(parseParagraphs(m_dataObject["text"].toObject()["traits"].toObject()["value"].toObject()["data"].toObject()["content"].toArray()));
    ui->equipmentEdit->setHtml(parseParagraphs(m_dataObject["text"].toObject()["equipment"].toObject()["value"].toObject()["data"].toObject()["content"].toArray()));
    ui->featuresEdit->setHtml(parseParagraphs(m_dataObject["text"].toObject()["features"].toObject()["value"].toObject()["data"].toObject()["content"].toArray()));
    ui->alliesEdit->setHtml(parseParagraphs(m_dataObject["text"].toObject()["allies"].toObject()["value"].toObject()["data"].toObject()["content"].toArray()));
    ui->personalityEdit->setHtml(parseParagraphs(m_dataObject["text"].toObject()["personality"].toObject()["value"].toObject()["data"].toObject()["content"].toArray()));
    ui->backgroundEdit->setHtml(parseParagraphs(m_dataObject["text"].toObject()["background"].toObject()["value"].toObject()["data"].toObject()["content"].toArray()));
    ui->questsEdit->setHtml(parseParagraphs(m_dataObject["text"].toObject()["quests"].toObject()["value"].toObject()["data"].toObject()["content"].toArray()));
    ui->idealsEdit->setHtml(parseParagraphs(m_dataObject["text"].toObject()["ideals"].toObject()["value"].toObject()["data"].toObject()["content"].toArray()));
    ui->bondsEdit->setHtml(parseParagraphs(m_dataObject["text"].toObject()["bonds"].toObject()["value"].toObject()["data"].toObject()["content"].toArray()));
    ui->flawsEdit->setHtml(parseParagraphs(m_dataObject["text"].toObject()["flaws"].toObject()["value"].toObject()["data"].toObject()["content"].toArray()));

    parseNotes();
    attackModel->fromJson(m_dataObject["weaponsList"].toArray());
    resourceModel->fromJson(m_dataObject["resources"].toObject());
}

QJsonObject DndCharsheetWidget::collectData(const QString& filePath) {
    QJsonObject result = m_dataObject;

    // name
    result["name"] = QJsonObject{
            {"value", ui->nameEdit->text()}
    };

    // info
    QJsonObject info;
    info["charClass"] = QJsonObject{{"value", ui->classEdit->text().split(" (").first()}};
    info["charSubclass"] = QJsonObject{{"value", ui->classEdit->text().split(" (").value(1).chopped(1)}};
    info["level"] = QJsonObject{{"value", ui->levelBox->value()}};
    info["playerName"] = QJsonObject{{"value", "Technohamster"}};  // при необходимости
    info["background"] = QJsonObject{{"value", result["info"].toObject().value("background").toObject().value("value")}};
    info["race"] = QJsonObject{{"value", result["info"].toObject().value("race").toObject().value("value")}};
    info["alignment"] = QJsonObject{{"value", result["info"].toObject().value("alignment").toObject().value("value")}};
    info["experience"] = QJsonObject{{"value", ""}};
    result["info"] = info;

    // vitality
    QJsonObject vitality;
    vitality["speed"] = QJsonObject{{"value", ui->speedBox->text()}};
    vitality["ac"] = QJsonObject{{"value", ui->acBox->value()}};
    vitality["hp-current"] = QJsonObject{{"value", ui->hpSpinBox->value()}};
    vitality["hp-max"] = QJsonObject{{"value", ui->maxHpBox->value()}};
    result["vitality"] = vitality;

    // stats
    auto statObject = [&](const QString& statName, int score) {
        return QJsonObject{{"score", score}, {"modifier", bonusFromStat(score)}};
    };

    QJsonObject stats;
    stats["str"] = statObject("str", ui->strValueEdit->value());
    stats["dex"] = statObject("dex", ui->dexValueEdit->value());
    stats["con"] = statObject("con", ui->conValueEdit->value());
    stats["int"] = statObject("int", ui->intValueEdit->value());
    stats["wis"] = statObject("wis", ui->wisValueEdit->value());
    stats["cha"] = statObject("cha", ui->chaValueEdit->value());
    result["stats"] = stats;

    // saves
    QJsonObject saves;
    saves["str"] = QJsonObject{{"isProf", ui->strSaveCheckBox->isChecked()}};
    saves["dex"] = QJsonObject{{"isProf", ui->dexSaveCheckBox->isChecked()}};
    saves["con"] = QJsonObject{{"isProf", ui->conSaveCheckBox->isChecked()}};
    saves["int"] = QJsonObject{{"isProf", ui->intSaveCheckBox->isChecked()}};
    saves["wis"] = QJsonObject{{"isProf", ui->wisSaveCheckBox->isChecked()}};
    saves["cha"] = QJsonObject{{"isProf", ui->chaSaveCheckBox->isChecked()}};
    result["saves"] = saves;

    // skills
    auto skill = [&](const QString& name, QCheckBox *checkBox, const QString& stat = "") {
        QJsonObject obj;
        obj["name"] = name;
        if (!stat.isEmpty()) obj["baseStat"] = stat;
        if (checkBox->isChecked()) obj["isProf"] = 1;
        return obj;
    };

    QJsonObject skills;
    skills["acrobatics"] = skill("acrobatics", ui->acrobatics, "dex");
    skills["athletics"] = skill("athletics", ui->athlecicsCheckBox, "str");
    skills["sleight of hand"] = skill("sleight of hand", ui->sleight, "dex");
    skills["stealth"] = skill("stealth", ui->stealth, "dex");
    skills["arcana"] = skill("arcana", ui->arcana, "int");
    skills["history"] = skill("history", ui->history, "int");
    skills["investigation"] = skill("investigation", ui->investigation, "int");
    skills["nature"] = skill("nature", ui->nature, "int");
    skills["religion"] = skill("religion", ui->religion, "int");
    skills["animal handling"] = skill("animal handling", ui->handling, "wis");
    skills["insight"] = skill("insight", ui->insight, "wis");
    skills["medicine"] = skill("medicine", ui->medicine, "wis");
    skills["perception"] = skill("perception", ui->perception, "wis");
    skills["survival"] = skill("survival", ui->survival, "wis");
    skills["deception"] = skill("deception", ui->deception, "cha");
    skills["intimidation"] = skill("intimidation", ui->intimidation, "cha");
    skills["performance"] = skill("performance", ui->performance, "cha");
    skills["persuasion"] = skill("persuasion", ui->persuasion, "cha");
    result["skills"] = skills;

    // текстовые блоки
    auto toDoc = [&](const QString &html) {
        return QJsonObject{
                {"value", QJsonObject{
                        {"data", QJsonObject{
                                {"type", "doc"},
                                {"content", serializeHtmlToJson(html)}  // сериализация HTML в JSON
                        }}
                }}
        };
    };
    QJsonObject text;
    text["prof"] = toDoc(ui->proficienciesEdit->toHtml());
    text["traits"] = toDoc(ui->traitsEdit->toHtml());
    text["equipment"] = toDoc(ui->equipmentEdit->toHtml());
    text["features"] = toDoc(ui->featuresEdit->toHtml());
    text["allies"] = toDoc(ui->alliesEdit->toHtml());
    text["personality"] = toDoc(ui->personalityEdit->toHtml());
    text["background"] = toDoc(ui->backgroundEdit->toHtml());
    text["quests"] = toDoc(ui->questsEdit->toHtml());
    text["ideals"] = toDoc(ui->idealsEdit->toHtml());
    text["bonds"] = toDoc(ui->bondsEdit->toHtml());
    text["flaws"] = toDoc(ui->flawsEdit->toHtml());
    result["text"] = text;

    // оружие и ресурсы
    result["weaponsList"] = attackModel->toJson();
    result["resources"] = resourceModel->toJson();

    return result;
}

/**
 * @brief Establishes connections between UI elements and corresponding functionalities.
 *
 * This function links the value changes and toggles of various UI elements, such as QSpinBox and QCheckBox,
 * to appropriate methods or lambdas. It ensures that changes in character attributes, skills, and proficiency
 * are dynamically reflected on the interface.
 *
 * Connections include:
 * - Update of proficiency bonus and dependent checkboxes when the level is adjusted.
 * - Handling updates to skill checkboxes and saving throws based on the corresponding ability scores.
 * - Synchronizing the display of related character data when associated widgets are toggled or modified.
 *
 * The function plays a key role in maintaining the responsiveness and accuracy of the character sheet widget.
 */
void DndCharsheetWidget::connectSignals() {
    connect(ui->levelBox, &QSpinBox::valueChanged, this, [=](){
        ui->proficiencyLabel->setText(QString::number(proficiencyByLevel(ui->levelBox->value())));
        updateCheckBoxes();
    });

    connect(ui->strValueEdit, &QSpinBox::valueChanged, this, [=](){updateCheckBoxes();});
    connect(ui->dexValueEdit, &QSpinBox::valueChanged, this, [=](){updateCheckBoxes();});
    connect(ui->conValueEdit, &QSpinBox::valueChanged, this, [=](){updateCheckBoxes();});
    connect(ui->intValueEdit, &QSpinBox::valueChanged, this, [=](){updateCheckBoxes();});
    connect(ui->wisValueEdit, &QSpinBox::valueChanged, this, [=](){updateCheckBoxes();});
    connect(ui->chaValueEdit, &QSpinBox::valueChanged, this, [=](){updateCheckBoxes();});

    connect(ui->strSaveCheckBox, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->strSaveCheckBox, ui->strValueEdit);});
    connect(ui->strSaveCheckBox, &TextClickableCheckBox::textClicked, [=](){emit rollRequested(QString("d20 %1").arg(bonusFromString(ui->strSaveCheckBox->text())));});
    connect(ui->dexSaveCheckBox, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->dexSaveCheckBox, ui->dexValueEdit);});
    connect(ui->dexSaveCheckBox, &TextClickableCheckBox::textClicked, [=](){emit rollRequested(QString("d20 %1").arg(bonusFromString(ui->dexSaveCheckBox->text())));});
    connect(ui->conSaveCheckBox, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->conSaveCheckBox, ui->conValueEdit);});
    connect(ui->conSaveCheckBox, &TextClickableCheckBox::textClicked, [=](){emit rollRequested(QString("d20 %1").arg(bonusFromString(ui->conSaveCheckBox->text())));});
    connect(ui->intSaveCheckBox, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->intSaveCheckBox, ui->intValueEdit);});
    connect(ui->intSaveCheckBox, &TextClickableCheckBox::textClicked, [=](){emit rollRequested(QString("d20 %1").arg(bonusFromString(ui->intSaveCheckBox->text())));});
    connect(ui->wisSaveCheckBox, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->wisSaveCheckBox, ui->wisValueEdit);});
    connect(ui->wisSaveCheckBox, &TextClickableCheckBox::textClicked, [=](){emit rollRequested(QString("d20 %1").arg(bonusFromString(ui->wisSaveCheckBox->text())));});
    connect(ui->chaSaveCheckBox, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->chaSaveCheckBox, ui->chaValueEdit);});
    connect(ui->chaSaveCheckBox, &TextClickableCheckBox::textClicked, [=](){emit rollRequested(QString("d20 %1").arg(bonusFromString(ui->chaSaveCheckBox->text())));});

    connect(ui->athlecicsCheckBox, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->athlecicsCheckBox, ui->strValueEdit);});
    connect(ui->athlecicsCheckBox, &TextClickableCheckBox::textClicked, [=](){emit rollRequested(QString("d20 %1").arg(bonusFromString(ui->athlecicsCheckBox->text())));});

    connect(ui->acrobatics, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->acrobatics, ui->dexValueEdit);});
    connect(ui->acrobatics, &TextClickableCheckBox::textClicked, [=](){emit rollRequested(QString("d20 %1").arg(bonusFromString(ui->acrobatics->text())));});
    connect(ui->sleight, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->sleight, ui->dexValueEdit);});
    connect(ui->sleight, &TextClickableCheckBox::textClicked, [=](){emit rollRequested(QString("d20 %1").arg(bonusFromString(ui->sleight->text())));});
    connect(ui->stealth, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->stealth, ui->dexValueEdit);});
    connect(ui->stealth, &TextClickableCheckBox::textClicked, [=](){emit rollRequested(QString("d20 %1").arg(bonusFromString(ui->stealth->text())));});


    connect(ui->arcana, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->arcana, ui->intValueEdit);});
    connect(ui->arcana, &TextClickableCheckBox::textClicked, [=](){emit rollRequested(QString("d20 %1").arg(bonusFromString(ui->arcana->text())));});
    connect(ui->history, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->history, ui->intValueEdit);});
    connect(ui->history, &TextClickableCheckBox::textClicked, [=](){emit rollRequested(QString("d20 %1").arg(bonusFromString(ui->history->text())));});
    connect(ui->investigation, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->investigation, ui->intValueEdit);});
    connect(ui->investigation, &TextClickableCheckBox::textClicked, [=](){emit rollRequested(QString("d20 %1").arg(bonusFromString(ui->investigation->text())));});
    connect(ui->nature, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->nature, ui->intValueEdit);});
    connect(ui->nature, &TextClickableCheckBox::textClicked, [=](){emit rollRequested(QString("d20 %1").arg(bonusFromString(ui->nature->text())));});
    connect(ui->religion, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->religion, ui->intValueEdit);});
    connect(ui->religion, &TextClickableCheckBox::textClicked, [=](){emit rollRequested(QString("d20 %1").arg(bonusFromString(ui->religion->text())));});

    connect(ui->handling, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->handling, ui->wisValueEdit);});
    connect(ui->handling, &TextClickableCheckBox::textClicked, [=](){emit rollRequested(QString("d20 %1").arg(bonusFromString(ui->handling->text())));});
    connect(ui->insight, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->insight, ui->wisValueEdit);});
    connect(ui->insight, &TextClickableCheckBox::textClicked, [=](){emit rollRequested(QString("d20 %1").arg(bonusFromString(ui->insight->text())));});
    connect(ui->medicine, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->medicine, ui->wisValueEdit);});
    connect(ui->medicine, &TextClickableCheckBox::textClicked, [=](){emit rollRequested(QString("d20 %1").arg(bonusFromString(ui->medicine->text())));});
    connect(ui->perception, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->perception, ui->wisValueEdit);});
    connect(ui->perception, &TextClickableCheckBox::textClicked, [=](){emit rollRequested(QString("d20 %1").arg(bonusFromString(ui->perception->text())));});
    connect(ui->survival, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->survival, ui->wisValueEdit);});
    connect(ui->survival, &TextClickableCheckBox::textClicked, [=](){emit rollRequested(QString("d20 %1").arg(bonusFromString(ui->survival->text())));});

    connect(ui->deception, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->deception, ui->chaValueEdit);});
    connect(ui->deception, &TextClickableCheckBox::textClicked, [=](){emit rollRequested(QString("d20 %1").arg(bonusFromString(ui->deception->text())));});
    connect(ui->intimidation, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->intimidation, ui->chaValueEdit);});
    connect(ui->intimidation, &TextClickableCheckBox::textClicked, [=](){emit rollRequested(QString("d20 %1").arg(bonusFromString(ui->intimidation->text())));});
    connect(ui->performance, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->performance, ui->chaValueEdit);});
    connect(ui->performance, &TextClickableCheckBox::textClicked, [=](){emit rollRequested(QString("d20 %1").arg(bonusFromString(ui->performance->text())));});
    connect(ui->persuasion, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->persuasion, ui->chaValueEdit);});
    connect(ui->persuasion, &TextClickableCheckBox::textClicked, [=](){emit rollRequested(QString("d20 %1").arg(bonusFromString(ui->persuasion->text())));});

    connect(ui->strLabel, &ClickableLabel::clicked, [=](){emit rollRequested(QString("d20 %1").arg((ui->strBonusLabel->text().startsWith("-")) ? ui->strBonusLabel->text() : "+" + ui->strBonusLabel->text()));});
    connect(ui->dexLabel, &ClickableLabel::clicked, [=](){emit rollRequested(QString("d20 %1").arg((ui->dexBonusLabel->text().startsWith("-")) ? ui->dexBonusLabel->text() : "+" + ui->dexBonusLabel->text()));});
    connect(ui->conLabel, &ClickableLabel::clicked, [=](){emit rollRequested(QString("d20 %1").arg((ui->conBonusLabel->text().startsWith("-")) ? ui->conBonusLabel->text() : "+" + ui->conBonusLabel->text()));});
    connect(ui->intLanel, &ClickableLabel::clicked, [=](){emit rollRequested(QString("d20 %1").arg((ui->intBonusLabel->text().startsWith("-")) ? ui->intBonusLabel->text() : "+" + ui->intBonusLabel->text()));});
    connect(ui->wisLabel, &ClickableLabel::clicked, [=](){emit rollRequested(QString("d20 %1").arg((ui->wisBonusLabel->text().startsWith("-")) ? ui->wisBonusLabel->text() : "+" + ui->wisBonusLabel->text()));});
    connect(ui->chaLabel, &ClickableLabel::clicked, [=](){emit rollRequested(QString("d20 %1").arg((ui->chaBonusLabel->text().startsWith("-")) ? ui->chaBonusLabel->text() : "+" + ui->chaBonusLabel->text()));});

    connect(ui->addResourceButton, &QPushButton::clicked, [=](){
        ResourceDialog resourceDialog(this);
        if (resourceDialog.exec() == QDialog::Accepted){
            Resource newResource = resourceDialog.getCreatedResource();
            resourceModel->addResource(newResource);
        }
    });

    connect(ui->addAttackButton, &QPushButton::clicked, [=](){
        AttackDialog attackDialog(this);
        if (attackDialog.exec()== QDialog::Accepted){
            Attack attack = attackDialog.getCreatedAttack();
            attackModel->addAttack(attack);
        }
    });

    connect(ui->longRestButton, &QPushButton::clicked, resourceModel, &DndResourceModel::doLongRest);
    connect(ui->shortRestButton, &QPushButton::clicked, resourceModel, &DndResourceModel::doShortRest);
}

/**
 * @brief Updates the text of a QCheckBox to display a calculated bonus.
 *
 * This function recalculates and updates the text of a given QCheckBox based on the value of
 * a corresponding QSpinBox and whether the checkbox is checked. If the checkbox is checked,
 * the proficiency bonus is included in the calculation. The updated text will display the
 * original text of the checkbox, followed by the calculated bonus.
 *
 * @param checkBox The QCheckBox whose text is to be updated. The text should already include
 *        a label, with a numerical bonus appended using ":".
 * @param baseSpinBox The QSpinBox containing the base value used to calculate the bonus.
 *        The base value is processed using the `bonusFromStat` function.
 */
void DndCharsheetWidget::updateCheckBox(QCheckBox *checkBox, QSpinBox *baseSpinBox) {
    int profBonus = 0;
    if (checkBox->isChecked()) profBonus = ui->proficiencyLabel->text().toInt();
    checkBox->setText(QString("%1: %2").arg(checkBox->text().split(":").value(0), QString::number(bonusFromStat(baseSpinBox->value()) + profBonus)));
}

/**
 * Updates the text and state of multiple QCheckBox widgets based on corresponding QSpinBox values
 * from the user interface. This function delegates the update logic to the updateCheckBox method
 * for each relevant checkbox-spinbox pair.
 *
 * The function is intended to synchronize ability modifiers, proficiency bonuses, and skill checkboxes
 * in the character sheet interface based on the current user inputs or data.
 *
 * List of updates performed:
 * - Updates saving throw checkboxes for all abilities (Strength, Dexterity, Constitution, Intelligence, Wisdom, Charisma).
 * - Updates all listed skills' checkboxes for respective abilities such as Strength (Athletics), Dexterity (Acrobatics, Sleight of Hand, Stealth),
 *   Intelligence (Arcana, History, Investigation, Nature, Religion), Wisdom (Animal Handling, Insight, Medicine, Perception, Survival),
 *   and Charisma (Deception, Intimidation, Performance, Persuasion).
 *
 * This function ensures that the displayed text for each checkbox reflects the correct ability modifier
 * and includes the proficiency bonus if the checkbox is checked.
 *
 * Relies on:
 * - The updateCheckBox method for performing individual checkbox updates.
 * - UI elements (checkboxes and spinboxes) being correctly initialized and accessible via the `ui` member.
 */
void DndCharsheetWidget::updateCheckBoxes() {
    ui->strBonusLabel->setText(QString::number(bonusFromStat(ui->strValueEdit->value())));
    ui->dexBonusLabel->setText(QString::number(bonusFromStat(ui->dexValueEdit->value())));
    ui->conBonusLabel->setText(QString::number(bonusFromStat(ui->conValueEdit->value())));
    ui->intBonusLabel->setText(QString::number(bonusFromStat(ui->intValueEdit->value())));
    ui->wisBonusLabel->setText(QString::number(bonusFromStat(ui->wisValueEdit->value())));
    ui->chaBonusLabel->setText(QString::number(bonusFromStat(ui->chaValueEdit->value())));

    ui->initiativeLabel->setText(ui->dexBonusLabel->text());

    attackModel->setStrBonus(bonusFromStat(ui->strValueEdit->value()));
    attackModel->setDexBonus(bonusFromStat(ui->dexValueEdit->value()));
    attackModel->setConBonus(bonusFromStat(ui->conValueEdit->value()));
    attackModel->setIntBonus(bonusFromStat(ui->intValueEdit->value()));
    attackModel->setWisBonus(bonusFromStat(ui->wisValueEdit->value()));
    attackModel->setChaBonus(bonusFromStat(ui->chaValueEdit->value()));
    attackModel->setProfBonus(ui->proficiencyLabel->text().toInt());

    updateCheckBox(ui->strSaveCheckBox, ui->strValueEdit);
    updateCheckBox(ui->dexSaveCheckBox, ui->dexValueEdit);
    updateCheckBox(ui->conSaveCheckBox, ui->conValueEdit);
    updateCheckBox(ui->intSaveCheckBox, ui->intValueEdit);
    updateCheckBox(ui->wisSaveCheckBox, ui->wisValueEdit);
    updateCheckBox(ui->chaSaveCheckBox, ui->chaValueEdit);

    updateCheckBox(ui->athlecicsCheckBox, ui->strValueEdit);

    updateCheckBox(ui->acrobatics, ui->dexValueEdit);
    updateCheckBox(ui->sleight, ui->dexValueEdit);
    updateCheckBox(ui->stealth, ui->dexValueEdit);

    updateCheckBox(ui->arcana, ui->intValueEdit);
    updateCheckBox(ui->history, ui->intValueEdit);
    updateCheckBox(ui->investigation, ui->intValueEdit);
    updateCheckBox(ui->nature, ui->intValueEdit);
    updateCheckBox(ui->religion, ui->intValueEdit);

    updateCheckBox(ui->handling, ui->wisValueEdit);
    updateCheckBox(ui->insight, ui->wisValueEdit);
    updateCheckBox(ui->medicine, ui->wisValueEdit);
    updateCheckBox(ui->perception, ui->wisValueEdit);
    updateCheckBox(ui->survival, ui->wisValueEdit);

    updateCheckBox(ui->deception, ui->chaValueEdit);
    updateCheckBox(ui->intimidation, ui->chaValueEdit);
    updateCheckBox(ui->performance, ui->chaValueEdit);
    updateCheckBox(ui->persuasion, ui->chaValueEdit);
}

/**
 * @brief Adds the character associated with this widget to an initiative tracker.
 *
 * This method retrieves character details such as name, maximum hit points (HP),
 * armor class (AC), and current hit points (HP) from the user interface (UI) of the
 * DndCharsheetWidget instance and passes them to the provided InitiativeTrackerWidget
 * for inclusion in the initiative tracker.
 *
 * @param initiativeTrackerWidget A pointer to an instance of InitiativeTrackerWidget where the character will be added.
 */
void DndCharsheetWidget::addToInitiative(InitiativeTrackerWidget *initiativeTrackerWidget, bool autoRoll) {
    int initiative = 0;
    if (autoRoll)
        initiative = rollDice(20) + ui->initiativeLabel->text().toInt();
    initiativeTrackerWidget->addCharacter(ui->nameEdit->text(), ui->maxHpBox->value(), ui->acBox->value(), ui->hpSpinBox->value(), initiative, ui->speedBox->value());
}

QString DndCharsheetWidget::parseParagraphs(const QJsonArray &content) {
    QString result;
    for (const auto& paragraphVal : content) {
        QJsonObject paragraph = paragraphVal.toObject();
        if (!(paragraph["type"].toString() == "paragraph"))
            continue;
        QJsonArray parts = paragraph["content"].toArray();
        QString line;
        for (const auto& partVal: parts) {
            QJsonObject part = partVal.toObject();
            QString text = part["text"].toString();
            if (text.isEmpty()) continue;

            if (part.contains("marks")){
                QJsonArray marks = part["marks"].toArray();
                for (const auto& markVal : marks) {
                    QString markType = markVal.toObject()["type"].toString();

                    if (markType == "bold")
                        text = "<b>" + text + "</b>";
                    else if (markType == "italic")
                        text = "<i>" + text + "</i>";
                    else if (markType == "underline")
                        text = "<u>" + text + "</u>";
                }
            }
            line += text;
        }
        if (!line.isEmpty())
            result += "<p>" + line + "</p>";
    }
    return result.trimmed();
}

void DndCharsheetWidget::parseNotes() {
    QString result;

    result += parseParagraphs(m_dataObject["text"].toObject()["notes-1"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result += parseParagraphs(m_dataObject["text"].toObject()["notes-2"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result += parseParagraphs(m_dataObject["text"].toObject()["notes-3"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result += parseParagraphs(m_dataObject["text"].toObject()["notes-4"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result += parseParagraphs(m_dataObject["text"].toObject()["notes-5"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result += parseParagraphs(m_dataObject["text"].toObject()["notes-6"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());

    ui->notesEdit->setHtml(result);
}

void DndCharsheetWidget::setupShortcuts() {
    m_boldShortcut = new QShortcut(QKeySequence("Ctrl+B"), this);
    connect(m_boldShortcut, &QShortcut::activated, [=](){
        QTextEdit *edit = getFocusedEdit();
        QTextCursor cursor = edit->textCursor();
        if (cursor.hasSelection()) {
            QTextCharFormat format;
            int weight = cursor.charFormat().fontWeight();
            format.setFontWeight(weight == QFont::Bold ? QFont::Normal : QFont::Bold);
            cursor.mergeCharFormat(format);
        }
    });

    m_italicShortcut = new QShortcut(QKeySequence("Ctrl+I"), this);
    connect(m_italicShortcut, &QShortcut::activated, [=](){
        QTextEdit *edit = getFocusedEdit();
        if (edit) {
            QTextCursor cursor = edit->textCursor();
            if (cursor.hasSelection()) {
                QTextCharFormat format;
                format.setFontItalic(!cursor.charFormat().fontItalic());
                cursor.mergeCharFormat(format);
            }
        }
    });

    m_underlineShortcut = new QShortcut(QKeySequence("Ctrl+U"), this);
    connect(m_underlineShortcut, &QShortcut::activated, [=](){
        QTextEdit *edit = getFocusedEdit();
        if (edit) {
            QTextCursor cursor = edit->textCursor();
            if (cursor.hasSelection()) {
                QTextCharFormat format;
                format.setFontUnderline(!cursor.charFormat().fontUnderline());
                cursor.mergeCharFormat(format);
            }
        }
    });
}

QTextEdit *DndCharsheetWidget::getFocusedEdit() {
    QWidget *focusWidget = QApplication::focusWidget();
    return qobject_cast<QTextEdit*>(focusWidget);
}


QJsonArray DndCharsheetWidget::serializeHtmlToJson(const QString &html) {
    QJsonArray contentArray;

    QTextDocument doc;
    doc.setHtml(html);

    for (QTextBlock block = doc.begin(); block.isValid(); block = block.next()) {
        QJsonObject paragraph;
        QJsonArray paragraphContent;

        if (block.textList()) {
            QTextList *list = block.textList();

            QJsonObject item;
            item["type"] = "paragraph";

            QJsonArray innerContent;
            for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it) {
                QTextFragment frag = it.fragment();
                if (!frag.isValid()) continue;

                QString fragText = frag.text();
                if (fragText.trimmed().isEmpty()) continue;

                QJsonObject textObject;
                textObject["type"] = "text";
                textObject["text"] = fragText;

                QJsonArray marks;
                QTextCharFormat fmt = frag.charFormat();
                if (fmt.fontWeight() == QFont::Bold)
                    marks.append(QJsonObject{{"type", "bold"}});
                if (fmt.fontItalic())
                    marks.append(QJsonObject{{"type", "italic"}});
                if (fmt.fontUnderline())
                    marks.append(QJsonObject{{"type", "underline"}});

                if (!marks.isEmpty())
                    textObject["marks"] = marks;

                innerContent.append(textObject);
            }

            item["content"] = innerContent;
            contentArray.append(item);
            continue;
        }

        paragraph["type"] = "paragraph";
        for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it) {
            QTextFragment frag = it.fragment();
            if (!frag.isValid()) continue;

            QString fragText = frag.text();
            if (fragText.trimmed().isEmpty()) continue;

            QJsonObject textObject;
            textObject["type"] = "text";
            textObject["text"] = fragText;

            QJsonArray marks;
            QTextCharFormat fmt = frag.charFormat();
            if (fmt.fontWeight() == QFont::Bold)
                marks.append(QJsonObject{{"type", "bold"}});
            if (fmt.fontItalic())
                marks.append(QJsonObject{{"type", "italic"}});
            if (fmt.fontUnderline())
                marks.append(QJsonObject{{"type", "underline"}});

            if (!marks.isEmpty())
                textObject["marks"] = marks;

            paragraphContent.append(textObject);
        }

        if (!paragraphContent.isEmpty()) {
            paragraph["content"] = paragraphContent;
            contentArray.append(paragraph);
        } else {
            // Добавим пустой параграф, если строка была пустой
            contentArray.append(QJsonObject{{"type", "paragraph"}});
        }
    }

    return contentArray;
}

void DndCharsheetWidget::saveToFile(QString path) {
    QJsonObject updatedData = collectData();

    QJsonDocument innerDoc(updatedData);
    QString jsonString = QString::fromUtf8(innerDoc.toJson(QJsonDocument::Compact));

    QJsonObject root = m_originalDocument.object();
    root["data"] = jsonString;

    QJsonDocument finalDoc(root);
    QFile fileOut(path);
    if (!fileOut.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    fileOut.write(finalDoc.toJson(QJsonDocument::Indented));
    fileOut.close();
}

void DndCharsheetWidget::closeEvent(QCloseEvent *event) {
    QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                              tr("Save changes"),
                                                              tr("Save changes?"),
                                                              QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (reply == QMessageBox::Cancel){
        event->ignore();
        return;
    }

    if (reply == QMessageBox::Yes)
        saveToFile(m_originalFilePath);

    QWidget::closeEvent(event);
}

QString DndCharsheetWidget::bonusFromString(const QString& string) {
    QString bonus = string.split(": ").value(1);
    return bonus.startsWith("-") ? bonus : "+" + bonus;
}

void DndCharsheetWidget::updateTranslator() {
    ui->retranslateUi(this);
}
