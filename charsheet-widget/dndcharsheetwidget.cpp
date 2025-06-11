#include "dndcharsheetwidget.h"
#include "ui_dndcharsheetwidget.h"

#include <QMessageBox>
#include <QStandardItemModel>


DndCharsheetWidget::DndCharsheetWidget(QWidget* parent) :
        AbstractCharsheetWidget(parent), ui(new Ui::DndCharsheetWidget) {
    ui->setupUi(this);

    setupShortcuts();
    connectSignals();

    attackModel = new DndAttackModel(this);
    ui->attacsView->setModel(attackModel);
    ui->attacsView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->attacsView->setDropIndicatorShown(true);

    connect(ui->attacsView, &QTableView::clicked, this, [=](const QModelIndex &index) {
        if (index.column() == 5) {
            attackModel->deleteWeapon(index.row());
        }
    });
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
void DndCharsheetWidget::loadFromFile(QString path) {
    m_originalFilePath = path;
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
    ui->nameLabel->setText(m_dataObject["name"].toObject()["value"].toString());


    QJsonObject infoObject = m_dataObject["info"].toObject();
    QString classString = QString("%1 (%2)").arg(infoObject["charClass"].toObject()["value"].toString(), infoObject["charSubclass"].toObject()["value"].toString());
    ui->classLabel->setText(classString);
    ui->levelBox->setValue(infoObject["level"].toObject()["value"].toInt());
    ui->proficiencyLabel->setText(QString::number(proficiencyByLevel(ui->levelBox->value())));


    QJsonObject vitalityObject = m_dataObject["vitality"].toObject();
    ui->runSpeedLabel->setText(vitalityObject["speed"].toObject()["value"].toString());
    ui->acLabel->setText(QString::number(vitalityObject["ac"].toObject()["value"].toInt()));
    ui->hpSpinBox->setValue(vitalityObject["hp-current"].toObject()["value"].toInt());
    ui->maxHpLabel->setText(QString::number(vitalityObject["hp-max"].toObject()["value"].toInt()));


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
}

void DndCharsheetWidget::saveToFile(QString filePath) {

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
    connect(ui->dexSaveCheckBox, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->dexSaveCheckBox, ui->dexValueEdit);});
    connect(ui->conSaveCheckBox, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->conSaveCheckBox, ui->conValueEdit);});
    connect(ui->intSaveCheckBox, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->intSaveCheckBox, ui->intValueEdit);});
    connect(ui->wisSaveCheckBox, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->wisSaveCheckBox, ui->wisValueEdit);});
    connect(ui->chaSaveCheckBox, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->chaSaveCheckBox, ui->chaValueEdit);});

    connect(ui->athlecicsCheckBox, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->athlecicsCheckBox, ui->strValueEdit);});

    connect(ui->acrobatics, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->acrobatics, ui->dexValueEdit);});
    connect(ui->sleight, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->sleight, ui->dexValueEdit);});
    connect(ui->stealth, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->stealth, ui->dexValueEdit);});


    connect(ui->arcana, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->arcana, ui->intValueEdit);});
    connect(ui->history, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->history, ui->intValueEdit);});
    connect(ui->investigation, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->investigation, ui->intValueEdit);});
    connect(ui->nature, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->nature, ui->intValueEdit);});
    connect(ui->religion, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->religion, ui->intValueEdit);});

    connect(ui->handling, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->handling, ui->wisValueEdit);});
    connect(ui->insight, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->insight, ui->wisValueEdit);});
    connect(ui->medicine, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->medicine, ui->wisValueEdit);});
    connect(ui->perception, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->perception, ui->wisValueEdit);});
    connect(ui->survival, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->survival, ui->wisValueEdit);});

    connect(ui->deception, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->deception, ui->chaValueEdit);});
    connect(ui->intimidation, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->intimidation, ui->chaValueEdit);});
    connect(ui->performance, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->performance, ui->chaValueEdit);});
    connect(ui->persuasion, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->persuasion, ui->chaValueEdit);});
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
void DndCharsheetWidget::addToInitiative(InitiativeTrackerWidget *initiativeTrackerWidget) {
    initiativeTrackerWidget->addCharacter(ui->nameLabel->text(), ui->maxHpLabel->text().toInt(), ui->acLabel->text().toInt(), ui->hpSpinBox->value());
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