#include "dndbestiarypage.h"
#include "ui_dndbestiarypage.h"

#include <QFile>
#include <QMessageBox>
#include <QJsonArray>


DndBestiaryPage::DndBestiaryPage(QWidget *parent) : AbstractCharsheetWidget(parent), ui(new Ui::DndBestiaryPage) {
    ui->setupUi(this);
    m_manager = new QNetworkAccessManager(this);
    connect(ui->infoField, &RollTextBrowser::rollRequested, [=](const QString& expr){emit rollRequested(expr);});
    connect(ui->skillsBrowser, &RollTextBrowser::rollRequested, [=](const QString& expr){emit rollRequested(expr);});
}

DndBestiaryPage::~DndBestiaryPage() {
    delete ui;
}

DndBestiaryPage::DndBestiaryPage(QString filePath, QWidget *parent) : AbstractCharsheetWidget(parent), ui(new Ui::DndBestiaryPage) {
    ui->setupUi(this);
    m_manager = new QNetworkAccessManager(this);
    connect(ui->infoField, &RollTextBrowser::rollRequested, [=](const QString& expr){emit rollRequested(expr);});
    connect(ui->skillsBrowser, &RollTextBrowser::rollRequested, [=](const QString& expr){emit rollRequested(expr);});

    loadFromFile(filePath);
}

void DndBestiaryPage::loadFromFile(const QString &path) {
    QFile beastFile(path);
    m_campaignPath = campaignDirFromFile(path);

    if (!beastFile.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this, "error", "Can't open bestiary file");
        return;
    }

    QJsonDocument document = QJsonDocument::fromJson(beastFile.readAll());
    QJsonObject root = document.object();

    if (!root["system"].isNull())
        parseFromFvtt11(root);
    else
        parseFromFvtt10(root);
}

QString DndBestiaryPage::convertToHeader(QString type) {
    if (! typeToHeader.keys().contains(type))
        return type;
    return typeToHeader[type];
}

void DndBestiaryPage::addToInitiative(InitiativeTrackerWidget *initiativeTrackerWidget, bool autoRoll) {
    int initiative = 0;
    if (autoRoll)
        initiative = rollDice(20) + ui->dexBonusLabel->text().toInt();
    initiativeTrackerWidget->addCharacter(ui->nameLabel->text(), ui->hpLabel->text().toInt(), ui->acLabel->text().toInt(), ui->hpLabel->text().toInt(), initiative);
}

void DndBestiaryPage::updateTranslator() {
    ui->retranslateUi(this);
}

bool DndBestiaryPage::downloadToken(const QString &link) {
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

    QString fullPath = dir.filePath("Tokens/" + filename);
    QFileInfo fi(fullPath);
    if (fi.exists()) {
//        qInfo() << "Token already exists:" << fullPath;
        setTokenPixmap(fullPath);
        return false;
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

void DndBestiaryPage::setTokenPixmap(const QString &filePath) {
    ui->token->setPixmap(QPixmap(filePath));
}

void DndBestiaryPage::parseFromFvtt11(QJsonObject root) {
    /// Name
    ui->nameLabel->setText(root.value("name").toString());

    QJsonObject systemObj = root["system"].toObject();

    QJsonObject attributesObj = systemObj["attributes"].toObject();


    /// Token
    downloadToken(root.value("img").toString());


    /// AC
    QJsonObject acObj = attributesObj["ac"].toObject();
    ui->acLabel->setText(QString::number(acObj.value("flat").toInt()));
    ui->acCalcLabel->setText(acObj.value("calc").toString());


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
    ui->strBonusLabel->setText(QString::number(bonusFromStat(abilitiesObj["str"].toObject().value("value").toInt())));
    saveBonus = ui->strBonusLabel->text().toInt() + ((abilitiesObj["str"].toObject().value("proficient").toInt() > 0) ? ui->profLabel->text().toInt() : 0);
    ui->strSaveBonus->setText((saveBonus > 0) ? "+" + QString::number(saveBonus) : QString::number(saveBonus));

    ui->dexValueLabel->setText(QString::number(abilitiesObj["dex"].toObject().value("value").toInt()));
    ui->dexBonusLabel->setText(QString::number(bonusFromStat(abilitiesObj["dex"].toObject().value("value").toInt())));
    saveBonus = ui->dexBonusLabel->text().toInt() + ((abilitiesObj["dex"].toObject().value("proficient").toInt() > 0) ? ui->profLabel->text().toInt() : 0);
    ui->dexSaveBonus->setText((saveBonus > 0) ? "+" + QString::number(saveBonus) : QString::number(saveBonus));

    ui->conValueLabel->setText(QString::number(abilitiesObj["con"].toObject().value("value").toInt()));
    ui->conBonusLabel->setText(QString::number(bonusFromStat(abilitiesObj["con"].toObject().value("value").toInt())));
    saveBonus = ui->conBonusLabel->text().toInt() + ((abilitiesObj["con"].toObject().value("proficient").toInt() > 0) ? ui->profLabel->text().toInt() : 0);
    ui->conSaveBonus->setText((saveBonus > 0) ? "+" + QString::number(saveBonus) : QString::number(saveBonus));

    ui->intValueLabel->setText(QString::number(abilitiesObj["int"].toObject().value("value").toInt()));
    ui->intBonusLabel->setText(QString::number(bonusFromStat(abilitiesObj["int"].toObject().value("value").toInt())));
    saveBonus = ui->intBonusLabel->text().toInt() + ((abilitiesObj["int"].toObject().value("proficient").toInt() > 0) ? ui->profLabel->text().toInt() : 0);
    ui->intSaveBonus->setText((saveBonus > 0) ? "+" + QString::number(saveBonus) : QString::number(saveBonus));

    ui->wisValueLabel->setText(QString::number(abilitiesObj["wis"].toObject().value("value").toInt()));
    ui->wisBonusLabel->setText(QString::number(bonusFromStat(abilitiesObj["wis"].toObject().value("value").toInt())));
    saveBonus = ui->wisBonusLabel->text().toInt() + ((abilitiesObj["wis"].toObject().value("proficient").toInt() > 0) ? ui->profLabel->text().toInt() : 0);
    ui->wisSaveBonus->setText((saveBonus > 0) ? "+" + QString::number(saveBonus) : QString::number(saveBonus));

    ui->chaValueLabel->setText(QString::number(abilitiesObj["cha"].toObject().value("value").toInt()));
    ui->chaBonusLabel->setText(QString::number(bonusFromStat(abilitiesObj["cha"].toObject().value("value").toInt())));
    saveBonus = ui->chaBonusLabel->text().toInt() + ((abilitiesObj["cha"].toObject().value("proficient").toInt() > 0) ? ui->profLabel->text().toInt() : 0);
    ui->chaSaveBonus->setText((saveBonus > 0) ? "+" + QString::number(saveBonus) : QString::number(saveBonus));


    /// Skills
    QJsonObject skillsObj = systemObj["skills"].toObject();
    QString skillsHtml = "";
    for (const auto& skillKey : skillTitles.keys()) {
        QJsonObject skillObj = skillsObj[skillKey].toObject();
        if (skillObj.value("value").toInt() <= 0)
            continue;

        QString skillAbility = skillObj.value("ability").toString();
        QString skillTitle = skillTitles[skillKey];
        int bonus = 0;
        QString skillBonus;

        if (skillAbility == "str")
            bonus = ui->strBonusLabel->text().toInt() + ui->profLabel->text().toInt();
        if (skillAbility == "dex")
            bonus = ui->dexBonusLabel->text().toInt() + ui->profLabel->text().toInt();
        if (skillAbility == "con")
            bonus = ui->conBonusLabel->text().toInt() + ui->profLabel->text().toInt();
        if (skillAbility == "int")
            bonus = ui->intBonusLabel->text().toInt() + ui->profLabel->text().toInt();
        if (skillAbility == "wis")
            bonus = ui->wisBonusLabel->text().toInt() + ui->profLabel->text().toInt();
        if (skillAbility == "cha")
            bonus = ui->chaBonusLabel->text().toInt() + ui->profLabel->text().toInt();

        skillBonus = (bonus > 0)? "+" + QString::number(bonus) : QString::number(bonus);

        skillsHtml += QString("%1: [[/r 1d20 %2]] (%2), ").arg(skillTitle, skillBonus);
    }
    ui->skillsBrowser->setCustomHtml(skillsHtml);


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

    ui->infoField->setCustomHtml(description);
}

void DndBestiaryPage::parseFromFvtt10(QJsonObject root) {

    /// Name
    ui->nameLabel->setText(root.value("name").toString());


    /// AC
    QJsonObject acObj = root["ac"].toObject();
    ui->acLabel->setText(QString::number(acObj.value("ac").toInt()));

    QJsonArray fromArray = acObj["from"].toArray();
    QString from = "";
    for (const auto& fromVal : fromArray) {
        from += fromVal.toString();
    }
    ui->acCalcLabel->setText(from);


    /// HP
    QJsonObject hpObj = root["hp"].toObject();
    ui->hpLabel->setText(QString::number(hpObj.value("average").toInt()));
    ui->hpFormula->setText(hpObj.value("formula").toString());


    /// Movement
    QJsonObject speedObj = root["speed"].toObject();
    QString speed = "";

    if (speedObj.value("walk").toInt() > 0)
        speed += tr("Walk: %1 %2").arg(QString::number(speedObj.value("walk").toInt()), "ft");

    if (speedObj.value("burrow").toInt() > 0)
        speed += tr(", Burrow: %1 %2").arg(QString::number(speedObj.value("burrow").toInt()), "ft");

    if (speedObj.value("climb").toInt() > 0)
        speed += tr(", climb: %1 %2").arg(QString::number(speedObj.value("climb").toInt()), "ft");

    if (speedObj.value("fly").toInt() > 0)
        speed += tr(", fly: %1 %2").arg(QString::number(speedObj.value("fly").toInt()), "ft");

    if (speedObj.value("swim").toInt() > 0)
        speed += tr(", swim: %1 %2").arg(QString::number(speedObj.value("swim").toInt()), "ft");

    ui->speedLabel->setText(speed);

    /// Senses
    QJsonArray sensesObj = root["senses"].toArray();
    QString senses = "";

    for (const auto& senseVal : sensesObj) {
        senses += senseVal.toString();
    }
    ui->fellingsLabel->setText(senses);


    /// Languages
    QJsonArray languagesList = root["languages"].toArray();
    QString languages = "";
    for (const auto& langVal: languagesList) {
        languages += langVal.toString();
    }
    ui->langLabel->setText(languages);


    /// Resistances and immunities
    QJsonArray damageImmunityList = root["immune"].toArray();
    QString damageImmunity = "";
    for (const auto& diVal : damageImmunityList) {
        damageImmunity += QString("%1, ").arg(diVal.toString());
    }
    ui->diLabel->setText(damageImmunity);

    QJsonArray damageResistanceList = root["resist"].toArray();
    QString damageResistance = "";
    for (const auto& drVal : damageResistanceList) {
        damageResistance += QString("%1, ").arg(drVal.toString());
    }
    ui->drLabel->setText(damageResistance);

    QJsonArray conditionImmunityList = root["conditionImmune"].toArray();
    QString conditionImmunity = "";
    for (const auto& ciVal : conditionImmunityList) {
        conditionImmunity += QString("%1, ").arg(ciVal.toString());
    }
    ui->ciLabel->setText(conditionImmunity);

    QJsonArray damageVulnerabilityList = root["Vulnerability"].toArray();
    QString damageVulnerability = "";
    for (const auto& ciVal : damageVulnerabilityList) {
        damageVulnerability += QString("%1, ").arg(ciVal.toString());
    }
    ui->dvLabel->setText(damageVulnerability);


    /// Abilities
    QJsonObject savesObj = root["save"].toObject();

    ui->strValueLabel->setText(QString::number(root.value("str").toInt()));
    ui->strBonusLabel->setText(QString::number(bonusFromStat(root.value("str").toInt())));
    ui->strSaveBonus->setText(savesObj.value("str").toString());

    ui->dexValueLabel->setText(QString::number(root.value("dex").toInt()));
    ui->dexBonusLabel->setText(QString::number(bonusFromStat(root.value("dex").toInt())));
    ui->dexSaveBonus->setText(savesObj.value("dex").toString());

    ui->conValueLabel->setText(QString::number(root.value("con").toInt()));
    ui->conBonusLabel->setText(QString::number(bonusFromStat(root.value("con").toInt())));
    ui->conSaveBonus->setText(savesObj.value("con").toString());

    ui->intValueLabel->setText(QString::number(root.value("int").toInt()));
    ui->intBonusLabel->setText(QString::number(bonusFromStat(root.value("int").toInt())));
    ui->intSaveBonus->setText(savesObj.value("int").toString());

    ui->wisValueLabel->setText(QString::number(root.value("wis").toInt()));
    ui->wisBonusLabel->setText(QString::number(bonusFromStat(root.value("wis").toInt())));
    ui->wisSaveBonus->setText(savesObj.value("wis").toString());

    ui->chaValueLabel->setText(QString::number(root.value("cha").toInt()));
    ui->chaBonusLabel->setText(QString::number(bonusFromStat(root.value("cha").toInt())));
    ui->chaSaveBonus->setText(savesObj.value("cha").toString());


    /// Skills
    QJsonObject skillsObj = root["skill"].toObject();
    QString skillsHtml = "";
    for (const auto& skillKey : skillTitles.keys()) {
        QString skillTitle = skillTitles[skillKey];
        QString skillBonus = skillsObj[skillTitle].toString();

        if (skillBonus.isEmpty())
            continue;

        skillsHtml += QString("%1: [[/r 1d20 %2]] (%2), ").arg(skillTitle, skillBonus);
    }
    ui->skillsBrowser->setCustomHtml(skillsHtml);


    /// Details

    QString race = root.value("type").toString();

    QJsonArray alignmentArray = root["alignment"].toArray();
    QString alignment= "";
    for (const auto& alignmentArrayVal: alignmentArray) {
        alignment += alignmentArrayVal.toString();
    }

    QString details = QString("%1, %2").arg(race, alignment);
    ui->detailsLabel->setText(details);

    ui->dangerLabel->setText(root.value("cr").toString());


    /// Description
    QStringList itemList = {"trait", "action", "legendary"};
    for (const auto & section :  itemList) {
        QJsonArray traits = root[section].toArray();

        for (const auto& itemVal : traits) {
            QJsonObject item = itemVal.toObject();
            BestiaryItem bestiaryItem;

            bestiaryItem.name = item.value("name").toString();

            QString desc = "";

            for (const auto & entry : item["entries"].toArray()) {
                desc += "<br>" + entry.toString();
            }

            desc.replace("&nbsp;", "");
            desc.replace("{@atk rs}", "");
            desc.replace("{@atk mw}", "");
            desc.replace("{@h};", "");
            desc.replace("{@h}", "");
            desc.replace("<hr />", " ");

            bestiaryItem.description = desc;

            bestiaryItem.type = section;

            descriptionSections[bestiaryItem.type].append(bestiaryItem);
        }
    }

    QString description;
    for (const auto& sectionTitle : descriptionSections.keys()){
        description += QString("<H1>%1</H1>").arg(convertToHeader(sectionTitle));
        for (const auto& item : descriptionSections[sectionTitle]) {
            description += QString("<H3>%1</H3>%2").arg(item.name, item.description);
        }
    }

    ui->infoField->setCustomHtml(description);
}
