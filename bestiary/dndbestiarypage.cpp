#include "dndbestiarypage.h"
#include "ui_dndbestiarypage.h"

#include <QFile>
#include <QMessageBox>
#include <QJsonArray>


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
    ui->acLabel->setText(QString::number(acObj.value("flat").toInt()) + " " + acObj.value("calc").toString());

    /// HP
    QJsonObject hpObj = attributesObj["hp"].toObject();
    ui->hpLabel->setText(QString::number(hpObj.value("max").toInt()));
    ui->hpFormula->setText(hpObj.value("formula").toString());

    /// Proficiency
    ui->profLabel->setText(QString::number(attributesObj.value("prof").toInt()));

    /// Movement
    QJsonObject movementObj = attributesObj["movement"].toObject();
    QString speed = "";
    QString units = movementObj.value("units").toString();

    if (movementObj.value("walk").toInt() > 0)
        speed += tr("Walk: %1 %2").arg(QString::number(movementObj.value("walk").toInt()), units);

    if (movementObj.value("burrow").toInt() > 0)
        speed += tr(", Burrow: %1 %2").arg(QString::number(movementObj.value("burrow").toInt()), units);

    if (movementObj.value("climb").toInt() > 0)
        speed += tr(", climb: %1 %2").arg(QString::number(movementObj.value("climb").toInt()), units);

    if (movementObj.value("fly").toInt() > 0)
        speed += tr(", fly: %1 %2").arg(QString::number(movementObj.value("fly").toInt()), units);

    if (movementObj.value("swim").toInt() > 0)
        speed += tr(", swim: %1 %2").arg(QString::number(movementObj.value("swim").toInt()), units);

    ui->speedLabel->setText(speed);

    /// Senses
    QJsonObject sensesObj = attributesObj["senses"].toObject();
    QString senses = "";
    units = sensesObj.value("units").toString();

    if (sensesObj.value("darkvision").toInt() > 0)
        senses += tr("Dark vision %1 %2").arg(QString::number(sensesObj.value("darkvision").toInt()), units);

    if (sensesObj.value("blindsight").toInt() > 0)
        senses += tr("Blindsight %1 %2").arg(QString::number(sensesObj.value("blindsight").toInt()), units);

    if (sensesObj.value("tremorsense").toInt() > 0)
        senses += tr("Tremorsense %1 %2").arg(QString::number(sensesObj.value("tremorsense").toInt()), units);

    if (sensesObj.value("truesight").toInt() > 0)
        senses += tr("Truesight %1 %2").arg(QString::number(sensesObj.value("truesight").toInt()), units);

    senses += sensesObj.value("special").toString();

    ui->fellingsLabel->setText(senses);


    /// Languages
    QJsonObject traitsObj = systemObj["traits"].toObject();
    QJsonArray languagesList = traitsObj["languages"].toObject()["value"].toArray();
    QString languages = "";
    for (const auto& langVal: languagesList) {
        languages += QString("%1, ").arg(langVal.toString());
    }
    languages += traitsObj["languages"].toObject().value("custom").toString();
    ui->langLabel->setText(languages);


    /// Resistances and immunities
    QJsonObject diObj = traitsObj["di"].toObject();
    QJsonArray damageImmunityList = diObj["value"].toArray();
    QString damageImmunity = "";
    for (const auto& diVal : damageImmunityList) {
        damageImmunity += QString("%1, ").arg(diVal.toString());
    }
    damageImmunity += diObj.value("custom").toString();
    ui->diLabel->setText(damageImmunity);

    QJsonObject drObj = traitsObj["dr"].toObject();
    QJsonArray damageResistanceList = drObj["value"].toArray();
    QString damageResistance = "";
    for (const auto& drVal : damageResistanceList) {
        damageResistance += QString("%1, ").arg(drVal.toString());
    }
    damageResistance += drObj.value("custom").toString();
    ui->drLabel->setText(damageResistance);

    QJsonObject ciObj = traitsObj["ci"].toObject();
    QJsonArray conditionImmunityList = ciObj["value"].toArray();
    QString conditionImmunity = "";
    for (const auto& ciVal : conditionImmunityList) {
        conditionImmunity += QString("%1, ").arg(ciVal.toString());
    }
    conditionImmunity += ciObj.value("custom").toString();
    ui->ciLabel->setText(conditionImmunity);

    QJsonObject dvObj = traitsObj["dv"].toObject();
    QJsonArray damageVulnerabilityList = dvObj["value"].toArray();
    QString damageVulnerability = "";
    for (const auto& ciVal : damageVulnerabilityList) {
        damageVulnerability += QString("%1, ").arg(ciVal.toString());
    }
    damageVulnerability += dvObj.value("custom").toString();
    ui->dvLabel->setText(damageVulnerability);
    
    /// Abilities
    QJsonObject abilitiesObj = systemObj["abilities"].toObject();
    int saveBonus;

    ui->strValueLabel->setText(QString::number(abilitiesObj["str"].toObject().value("value").toInt()));
    ui->strBonusLabel->setText(QString::number(abilitiesObj["str"].toObject().value("mod").toInt()));
    saveBonus = ui->strBonusLabel->text().toInt() + ((abilitiesObj["str"].toObject().value("proficient").toInt() > 0) ? ui->profLabel->text().toInt() : 0);
    ui->strSaveBonus->setText((saveBonus > 0) ? "+" + QString::number(saveBonus) : QString::number(saveBonus));

    ui->dexValueLabel->setText(QString::number(abilitiesObj["dex"].toObject().value("value").toInt()));
    ui->dexBonusLabel->setText(QString::number(abilitiesObj["dex"].toObject().value("mod").toInt()));
    saveBonus = ui->dexBonusLabel->text().toInt() + ((abilitiesObj["dex"].toObject().value("proficient").toInt() > 0) ? ui->profLabel->text().toInt() : 0);
    ui->dexSaveBonus->setText((saveBonus > 0) ? "+" + QString::number(saveBonus) : QString::number(saveBonus));

    ui->conValueLabel->setText(QString::number(abilitiesObj["con"].toObject().value("value").toInt()));
    ui->conBonusLabel->setText(QString::number(abilitiesObj["con"].toObject().value("mod").toInt()));
    saveBonus = ui->conBonusLabel->text().toInt() + ((abilitiesObj["con"].toObject().value("proficient").toInt() > 0) ? ui->profLabel->text().toInt() : 0);
    ui->conSaveBonus->setText((saveBonus > 0) ? "+" + QString::number(saveBonus) : QString::number(saveBonus));

    ui->intValueLabel->setText(QString::number(abilitiesObj["int"].toObject().value("value").toInt()));
    ui->intBonusLabel->setText(QString::number(abilitiesObj["int"].toObject().value("mod").toInt()));
    saveBonus = ui->intBonusLabel->text().toInt() + ((abilitiesObj["int"].toObject().value("proficient").toInt() > 0) ? ui->profLabel->text().toInt() : 0);
    ui->intSaveBonus->setText((saveBonus > 0) ? "+" + QString::number(saveBonus) : QString::number(saveBonus));

    ui->wisValueLabel->setText(QString::number(abilitiesObj["wis"].toObject().value("value").toInt()));
    ui->wisBonusLabel->setText(QString::number(abilitiesObj["wis"].toObject().value("mod").toInt()));
    saveBonus = ui->wisBonusLabel->text().toInt() + ((abilitiesObj["wis"].toObject().value("proficient").toInt() > 0) ? ui->profLabel->text().toInt() : 0);
    ui->wisSaveBonus->setText((saveBonus > 0) ? "+" + QString::number(saveBonus) : QString::number(saveBonus));

    ui->chaValueLabel->setText(QString::number(abilitiesObj["cha"].toObject().value("value").toInt()));
    ui->chaBonusLabel->setText(QString::number(abilitiesObj["cha"].toObject().value("mod").toInt()));
    saveBonus = ui->chaBonusLabel->text().toInt() + ((abilitiesObj["cha"].toObject().value("proficient").toInt() > 0) ? ui->profLabel->text().toInt() : 0);
    ui->chaSaveBonus->setText((saveBonus > 0) ? "+" + QString::number(saveBonus) : QString::number(saveBonus));

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

    ui->dangerLabel->setText(QString("%1 (%2 XP)").arg(QString::number(detailsObj.value("cr").toInt()), QString::number(detailsObj["xp"].toObject().value("value").toInt())));


    /// Description
    QJsonArray items = root["items"].toArray();

    for (const auto& itemVal : items) {
        QJsonObject item = itemVal.toObject();
        BestiaryItem bestiaryItem;

        bestiaryItem.name = item.value("name").toString();
        QString desc = item["system"].toObject().value("description").toObject().value("value").toString().split("<div class=\"rd__spc-inline-post\"></div>")[0];
        desc.replace(QRegularExpression(R"(@Compendium\[([^\]]+)\])"), "");
        desc.replace("<hr />", " ");
        desc.replace("{", "<u>");
        desc.replace("}", "</u>");
        bestiaryItem.description = desc;

        QString activationType = item["system"].toObject()["activation"].toObject().value("type").toString();
        if (activationType == "legendary" || activationType == "lair" || activationType == "action" && item.value("type").toString() != "spell")
            bestiaryItem.type = activationType;
        else
            bestiaryItem.type = item.value("type").toString();
        bestiaryItem.activationCost = item["system"].toObject()["activation"].toObject().value("cost").toInt();

        descriptionSections[bestiaryItem.type].append(bestiaryItem);
    }

    QString description;
    for (const auto& sectionTitle : descriptionSections.keys()){
        description += QString("<H1>%1</H1>").arg(convertToHeader(sectionTitle));
        for (const auto& item : descriptionSections[sectionTitle]) {
            description += QString("<H3>%1</H3>%2").arg(item.name, item.description);
        }
    }

    ui->infoField->setText(description);
}

QString DndBestiaryPage::convertToHeader(QString type) {
    if (! typeToHeader.keys().contains(type))
        return type;
    return typeToHeader[type];
}
