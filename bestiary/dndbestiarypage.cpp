#include "dndbestiarypage.h"
#include "ui_dndbestiarypage.h"

#include <QFile>
#include <QMessageBox>


DndBestiaryPage::DndBestiaryPage(QWidget *parent) :
        AbstractCharsheetWidget(parent), ui(new Ui::DndBestiaryPage) {
    ui->setupUi(this);
}

DndBestiaryPage::~DndBestiaryPage() {
    delete ui;
}

DndBestiaryPage::DndBestiaryPage(QString filePath, QWidget *parent) : AbstractCharsheetWidget(parent), ui(new Ui::DndBestiaryPage) {
    ui->setupUi(this);
    loadFromFile(filePath);
}

void DndBestiaryPage::loadFromFile(QString filePath) {
    QFile beastFile(filePath);
    if (!beastFile.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this, "error", "Can't open bestiary file");
        return;
    }

    QJsonDocument document = QJsonDocument::fromJson(beastFile.readAll());
    QJsonObject root = document.object();

    /// Name
    ui->nameLabel->setText(root.value("name").toString());

    QJsonObject systemObj = root["system"].toObject();

    QJsonObject attributesObj = systemObj["attributes"].toObject();

    /// AC
    QJsonObject acObj = attributesObj["ac"].toObject();
    ui->acLabel->setText(acObj.value("flat").toString() + acObj.value("calc").toString());

    /// HP
    QJsonObject hpObj = attributesObj["hp"].toObject();
    ui->hpLabel->setText(hpObj.value("max").toString());
    ui->hpFormula->setText(hpObj.value("formula").toString());

    /// Movement
    QJsonObject movementObj = attributesObj["movement"].toObject();
    QString speed = "";
    QString units = movementObj.value("units").toString();

    if (!movementObj.value("walk").toString().isEmpty())
        speed += tr("Walk: %1 %2").arg(movementObj.value("walk").toString(), units);

    if (!movementObj.value("burrow").toString().isEmpty())
        speed += tr(", Burrow: %1 %2").arg(movementObj.value("burrow").toString(), units);

    if (!movementObj.value("climb").toString().isEmpty())
        speed += tr("climb: %1 %2").arg(movementObj.value("climb").toString(), units);

    if (!movementObj.value("fly").toString().isEmpty())
        speed += tr("fly: %1 %2").arg(movementObj.value("fly").toString(), units);

    if (!movementObj.value("swim").toString().isEmpty())
        speed += tr("swim: %1 %2").arg(movementObj.value("swim").toString(), units);

    ui->speedLabel->setText(speed);

    /// Senses
    QJsonObject sensesObj = attributesObj["senses"].toObject();
    QString senses = "";
    units = sensesObj.value("units").toString();

    if (sensesObj.value("darkvision").toInt() > 0)
        senses += tr("Dark vision %1%2").arg(sensesObj.value("darkvision").toString(), units);

    if (sensesObj.value("blindsight").toInt() > 0)
        senses += tr("Blindsight %1%2").arg(sensesObj.value("blindsight").toString(), units);

    if (sensesObj.value("tremorsense").toInt() > 0)
        senses += tr("Tremorsense %1%2").arg(sensesObj.value("tremorsense").toString(), units);

    if (sensesObj.value("truesight").toInt() > 0)
        senses += tr("Truesight %1%2").arg(sensesObj.value("truesight").toString(), units);

    senses += sensesObj.value("special").toString();

    ui->fellingsLabel->setText(senses);

    /// Spell casting


    /// Abilities
    QJsonObject abilitiesObj = systemObj["abilities"].toObject();

    ui->strValueLabel->setText(abilitiesObj["str"].toObject().value("value").toString());
    ui->strBonusLabel->setText(abilitiesObj["str"].toObject().value("mod").toString());
    ui->strSaveBonus->setText(QString::number(ui->strBonusLabel->text().toInt() + ((abilitiesObj["str"].toObject().value("proficient").toBool()) ? ui->profLabel->text().toInt() : 0)));

    ui->dexValueLabel->setText(abilitiesObj["dex"].toObject().value("value").toString());
    ui->dexBonusLabel->setText(abilitiesObj["dex"].toObject().value("mod").toString());
    ui->dexSaveBonus->setText(QString::number(ui->strBonusLabel->text().toInt() + ((abilitiesObj["dex"].toObject().value("proficient").toBool()) ? ui->profLabel->text().toInt() : 0)));

    ui->conValueLabel->setText(abilitiesObj["con"].toObject().value("value").toString());
    ui->conBonusLabel->setText(abilitiesObj["con"].toObject().value("mod").toString());
    ui->conSaveBonus->setText(QString::number(ui->strBonusLabel->text().toInt() + ((abilitiesObj["con"].toObject().value("proficient").toBool()) ? ui->profLabel->text().toInt() : 0)));

    ui->intValueLabel->setText(abilitiesObj["int"].toObject().value("value").toString());
    ui->intBonusLabel->setText(abilitiesObj["int"].toObject().value("mod").toString());
    ui->intSaveBonus->setText(QString::number(ui->strBonusLabel->text().toInt() + ((abilitiesObj["int"].toObject().value("proficient").toBool()) ? ui->profLabel->text().toInt() : 0)));

    ui->wisValueLabel->setText(abilitiesObj["wis"].toObject().value("value").toString());
    ui->wisBonusLabel->setText(abilitiesObj["wis"].toObject().value("mod").toString());
    ui->wisSaveBonus->setText(QString::number(ui->strBonusLabel->text().toInt() + ((abilitiesObj["wis"].toObject().value("proficient").toBool()) ? ui->profLabel->text().toInt() : 0)));

    ui->chaValueLabel->setText(abilitiesObj["cha"].toObject().value("value").toString());
    ui->chaBonusLabel->setText(abilitiesObj["cha"].toObject().value("mod").toString());
    ui->chaSaveBonus->setText(QString::number(ui->strBonusLabel->text().toInt() + ((abilitiesObj["cha"].toObject().value("proficient").toBool()) ? ui->profLabel->text().toInt() : 0)));

    /// Details
    QJsonObject detailsObj = systemObj["details"].toObject();

    QString race;
    if (!detailsObj.value("race").toString().isEmpty())
        race = detailsObj.value("race").toString();
    else {
        QJsonObject typeObj = detailsObj["type"].toObject();
        race = QString("%1 [%2]").arg(typeObj.value("value").toString(), typeObj.value("subtype").toString());
    }

    QString details = QString("%1, %2").arg(race, detailsObj.value("alignment").toString());
    ui->detailsLabel->setText(details);

    ui->dangerLabel->setText(QString("%1 (%2 XP)").arg(detailsObj.value("cr").toString(), detailsObj["xp"].toObject().value("value").toString()));


    /// Description
    QJsonArray items = root["items"].toArray();
    QString description;

    for (const auto& itemVal : items) {
        QJsonObject item = itemVal.toObject();
        description += QString("<b>%1</b>: %2").arg(item.value("name").toString(), item["system"].toObject().value("value").toString());
    }
    ui->infoField->setText(description);
}
