#include "dndcharsheetwidget.h"
#include "ui_dndcharsheetwidget.h"

#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QStandardItemModel>

#include "dndcharsheetdialogs.h"
#include "characterparser.h"
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

    m_manager = new QNetworkAccessManager(this);

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
    delete m_manager;
    delete attackModel;
    delete resourceModel;
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
    characterFile.close();

    QJsonObject root = m_originalDocument.object();

    QString dataString = root.value("data").toString();
    m_dataObject = QJsonDocument::fromJson(dataString.toUtf8()).object();

    LssDndParser parser;
    populateWidget(parser.parseDnd(path));
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
void DndCharsheetWidget::populateWidget(const DndCharacterData& data) {
    ui->nameEdit->setText(data.name);


    ui->classEdit->setText(data.className);
    ui->levelBox->setValue(data.level);
    ui->proficiencyLabel->setText(QString::number(proficiencyByLevel(ui->levelBox->value())));


    ui->speedBox->setValue(data.speed);
    ui->acBox->setValue(data.ac);
    ui->hpSpinBox->setValue(data.hp);
    ui->maxHpBox->setValue(data.maxHp);

    ui->strValueEdit->setValue(data.stats["str"]);
    ui->strBonusLabel->setText(QString::number(bonusFromStat(ui->strValueEdit->value())));
    ui->strSaveCheckBox->setChecked(true);
    ui->strSaveCheckBox->setChecked(data.statProf["str"]);

    ui->dexValueEdit->setValue(data.stats["dex"]);
    ui->dexBonusLabel->setText(QString::number(bonusFromStat(ui->dexValueEdit->value())));
    ui->dexSaveCheckBox->setChecked(true);
    ui->dexSaveCheckBox->setChecked(data.statProf["dex"]);

    ui->conValueEdit->setValue(data.stats["con"]);
    ui->conBonusLabel->setText(QString::number(bonusFromStat(ui->conValueEdit->value())));
    ui->conSaveCheckBox->setChecked(true);
    ui->conSaveCheckBox->setChecked(data.statProf["con"]);

    ui->intValueEdit->setValue(data.stats["int"]);
    ui->intBonusLabel->setText(QString::number(bonusFromStat(ui->intValueEdit->value())));
    ui->intSaveCheckBox->setChecked(true);
    ui->intSaveCheckBox->setChecked(data.statProf["int"]);

    ui->wisValueEdit->setValue(data.stats["wis"]);
    ui->wisBonusLabel->setText(QString::number(bonusFromStat(ui->wisValueEdit->value())));
    ui->wisSaveCheckBox->setChecked(true);
    ui->wisSaveCheckBox->setChecked(data.statProf["wis"]);

    ui->chaValueEdit->setValue(data.stats["cha"]);
    ui->chaBonusLabel->setText(QString::number(bonusFromStat(ui->chaValueEdit->value())));
    ui->chaSaveCheckBox->setChecked(true);
    ui->chaSaveCheckBox->setChecked(data.statProf["cha"]);


    ui->athlecicsCheckBox->setChecked(true);
    ui->athlecicsCheckBox->setChecked(data.skillsProf["athletics"]);

    ui->acrobatics->setChecked(true);
    ui->acrobatics->setChecked(data.skillsProf["acrobatics"]);

    ui->sleight->setChecked(true);
    ui->sleight->setChecked(data.skillsProf["sleight of hand"]);

    ui->stealth->setChecked(true);
    ui->stealth->setChecked(data.skillsProf["stealth"]);

    ui->arcana->setChecked(true);
    ui->arcana->setChecked(data.skillsProf["arcana"]);

    ui->history->setChecked(true);
    ui->history->setChecked(data.skillsProf["history"]);

    ui->investigation->setChecked(true);
    ui->investigation->setChecked(data.skillsProf["investigation"]);

    ui->nature->setChecked(true);
    ui->nature->setChecked(data.skillsProf["nature"]);

    ui->religion->setChecked(true);
    ui->religion->setChecked(data.skillsProf["religion"]);

    ui->handling->setChecked(true);
    ui->handling->setChecked(data.skillsProf["animal handling"]);

    ui->insight->setChecked(true);
    ui->insight->setChecked(data.skillsProf["insight"]);

    ui->medicine->setChecked(true);
    ui->medicine->setChecked(data.skillsProf["medicine"]);

    ui->perception->setChecked(true);
    ui->perception->setChecked(data.skillsProf["perception"]);

    ui->survival->setChecked(true);
    ui->survival->setChecked(data.skillsProf["survival"]);

    ui->deception->setChecked(true);
    ui->deception->setChecked(data.skillsProf["deception"]);

    ui->intimidation->setChecked(true);
    ui->intimidation->setChecked(data.skillsProf["intimidation"]);

    ui->performance->setChecked(true);
    ui->performance->setChecked(data.skillsProf["performance"]);

    ui->persuasion->setChecked(true);
    ui->persuasion->setChecked(data.skillsProf["persuasion"]);



    ui->proficienciesEdit->setHtml(data.proficienciesHtml);
    ui->traitsEdit->setHtml(data.traitsHtml);
    ui->equipmentEdit->setHtml(data.equipmentHtml);
    ui->featuresEdit->setHtml(data.featuresHtml);
    ui->alliesEdit->setHtml(data.alliesHtml);
    ui->personalityEdit->setHtml(data.personalityHtml);
    ui->backgroundEdit->setHtml(data.backgroundHtml);
    ui->questsEdit->setHtml(data.questsHtml);
    ui->idealsEdit->setHtml(data.idealsHtml);
    ui->bondsEdit->setHtml(data.bondsHtml);
    ui->flawsEdit->setHtml(data.flawsHtml);

    ui->notesEdit->setHtml(data.notes);

    attackModel->fromJson(data.weapons);
    resourceModel->fromJson(data.resourcesObj);

    if (!data.tokenUrl.isEmpty())
        downloadToken(data.tokenUrl);
}

DndCharacterData DndCharsheetWidget::collectData() {
    DndCharacterData result;

    // name
    result.name = ui->nameEdit->text();

    // vitality
    result.speed = ui->speedBox->text().toInt();
    result.ac = ui->acBox->value();
    result.hp = ui->hpSpinBox->value();
    result.maxHp = ui->maxHpBox->value();


    result.stats["str"] = ui->strValueEdit->value();
    result.stats["dex"] = ui->dexValueEdit->value();
    result.stats["con"] = ui->conValueEdit->value();
    result.stats["int"] = ui->intValueEdit->value();
    result.stats["wis"] = ui->wisValueEdit->value();
    result.stats["cha"] = ui->chaValueEdit->value();

    // saves
    result.statProf["str"] = ui->strSaveCheckBox->isChecked();
    result.statProf["dex"] = ui->dexSaveCheckBox->isChecked();
    result.statProf["con"] = ui->conSaveCheckBox->isChecked();
    result.statProf["int"] = ui->intSaveCheckBox->isChecked();
    result.statProf["wis"] = ui->wisSaveCheckBox->isChecked();
    result.statProf["cha"] = ui->chaSaveCheckBox->isChecked();


    result.skillsProf["acrobatics"] = ui->acrobatics->isChecked();
    result.skillsProf["athletics"] = ui->athlecicsCheckBox->isChecked();
    result.skillsProf["sleight of hand"] = ui->sleight->isChecked();
    result.skillsProf["stealth"] = ui->stealth->isChecked();
    result.skillsProf["arcana"] = ui->arcana->isChecked();
    result.skillsProf["history"] = ui->history->isChecked();
    result.skillsProf["investigation"] = ui->investigation->isChecked();
    result.skillsProf["nature"] = ui->nature->isChecked();
    result.skillsProf["religion"] = ui->religion->isChecked();
    result.skillsProf["animal handling"] = ui->handling->isChecked();
    result.skillsProf["insight"] = ui->insight->isChecked();
    result.skillsProf["medicine"] = ui->medicine->isChecked();
    result.skillsProf["perception"] = ui->perception->isChecked();
    result.skillsProf["survival"] = ui->survival->isChecked();
    result.skillsProf["deception"] = ui->deception->isChecked();
    result.skillsProf["intimidation"] = ui->intimidation->isChecked();
    result.skillsProf["performance"] = ui->performance->isChecked();
    result.skillsProf["persuasion"] = ui->persuasion->isChecked();


    result.proficienciesHtml = ui->proficienciesEdit->toHtml();
    result.traitsHtml = ui->proficienciesEdit->toHtml();
    result.equipmentHtml = ui->equipmentEdit->toHtml();
    result.featuresHtml = ui->featuresEdit->toHtml();
    result.alliesHtml = ui->alliesEdit->toHtml();
    result.personalityHtml = ui->personalityEdit->toHtml();
    result.backgroundHtml = ui->backgroundEdit->toHtml();
    result.questsHtml = ui->questsEdit->toHtml();
    result.idealsHtml = ui->idealsEdit->toHtml();
    result.bondsHtml = ui->bondsEdit->toHtml();
    result.flawsHtml = ui->flawsEdit->toHtml();

    // оружие и ресурсы
    result.weapons = attackModel->toJson();
    result.resourcesObj = resourceModel->toJson();

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

/**
 * @brief Retrieves the currently focused QTextEdit widget.
 *
 * This method checks the widget that currently has input focus within
 * the application and returns it as a QTextEdit object if applicable.
 * It is used to identify the active text editor, enabling operations
 * like formatting or editing on that widget.
 *
 * @return A pointer to the focused QTextEdit, or nullptr if the focused
 * widget is not a QTextEdit.
 */
QTextEdit *DndCharsheetWidget::getFocusedEdit() {
    QWidget *focusWidget = QApplication::focusWidget();
    return qobject_cast<QTextEdit*>(focusWidget);
}


void DndCharsheetWidget::saveToFile(QString path) {
    LssDndParser parser;
    parser.writeDnd(collectData(), path);
}

/**
 * @brief Handles the close event for the widget.
 *
 * This method is triggered when the widget receives a close event.
 * It performs custom logic to ensure proper handling of unsaved changes
 * or other cleanup before the widget is closed.
 *
 * @param event A pointer to the QCloseEvent object containing details
 * about the close request.
 *
 * The method performs the following:
 * - Checks for unsaved changes or other conditions that might prevent
 *   the widget from closing.
 * - If the close request is rejected, the event is ignored, preventing
 *   the widget from being closed.
 * - If the close request is accepted, it executes any necessary cleanup
 *   logic before the widget is closed.
 */
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

/**
 * @brief Extracts and formats a bonus value from a given string.
 *
 * This method processes a string containing a key-value pair in the format
 * "key: bonus" and extracts the bonus value. It ensures that the returned
 * bonus value has a "+" sign prepended unless it already starts with a "-" sign.
 *
 * @param string A QString in the format of "key: bonus", where "bonus" is
 *        the numerical part to be extracted (positive or negative).
 * @return A formatted QString containing the extracted bonus value with
 *         a "+" sign for positive values or the original string for negative values.
 *
 * The method performs the following:
 * - Splits the input string on ": " to separate the key and bonus.
 * - Extracts the second part (bonus) from the split result.
 * - If the bonus does not start with a "-", prepends a "+" sign to it.
 * - Returns the modified bonus string.
 */
QString DndCharsheetWidget::bonusFromString(const QString& string) {
    QString bonus = string.split(": ").value(1);
    return bonus.startsWith("-") ? bonus : "+" + bonus;
}

/**
 * @brief Updates the application's translator for the specified language.
 *
 * This method reloads the translation files based on the provided language code.
 * It ensures that the application reflects the correct translations for UI elements
 * and user-facing text.
 *
 * @param languageCode A QString representing the desired language code (e.g., "en", "fr").
 *
 * The method performs the following:
 * - Removes any previously loaded translator instances.
 * - Loads the appropriate translation file corresponding to the specified language.
 * - Applies the loaded translator to the application to update the language dynamically.
 * - Displays a warning message if the translation file cannot be loaded.
 */
void DndCharsheetWidget::updateTranslator() {
    ui->retranslateUi(this);
}

bool DndCharsheetWidget::downloadToken(const QString &link) {
    QString fullPath = getTokenFileName(m_campaignPath, link);
    if (!fullPath.isEmpty()){
        setTokenPixmap(fullPath);
        return true;
    }

    QUrl qurl(link);
    if (!qurl.isValid()) {
        qWarning() << "Invalid URL:" << link;
        return false;
    }

    QString filename = qurl.fileName();
    if (filename.isEmpty()) {
//        qWarning() << "URL does not contain filename:" << link;
        return false;
    }

    QDir dir(m_campaignPath);
    if (!dir.exists()) {
//        qWarning() << "Campaign dir does not exist:" << m_campaignPath;
        return false;
    }

    // ensure tokens/ folder
    if (!dir.exists("Tokens")) {
        if (!dir.mkdir("Tokens")) {
            qWarning() << "Cannot create tokens dir!";
            return false;
        }
    }

    // async download
    QNetworkRequest req(qurl);
    auto reply = m_manager->get(req);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Download failed:" << reply->errorString();
            reply->deleteLater();
            return;
        }
        QByteArray data = reply->readAll();

        QFile f(fullPath);
        if (!f.open(QIODevice::WriteOnly)) {
            qWarning() << "Cannot write file:" << fullPath;
            reply->deleteLater();
            return;
        }
        f.write(data);
        f.close();
//        qInfo() << "Saved token:" << fullPath;
        setTokenPixmap(fullPath);
        reply->deleteLater();
    });
    return true;
}

void DndCharsheetWidget::setTokenPixmap(const QString &filePath) {
    ui->token->setPixmap(QPixmap(filePath));
}
