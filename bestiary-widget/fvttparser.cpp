#include "fvttparser.h"

BestiaryPageData Fvtt10Parser::parse(const QJsonObject &root) {
    BestiaryPageData data;

    data.name = root.value("name").toString();

    QJsonObject acObj = root["ac"].toObject();
    data.ac = QString::number(acObj.value("ac").toInt());

    QJsonArray fromArray = acObj["from"].toArray();
    QString from = "";
    for (const auto& fromVal : fromArray) {
        from += fromVal.toString();
    }
    data.acCalc = from;


    /// HP
    QJsonObject hpObj = root["hp"].toObject();
    data.hp = QString::number(hpObj.value("average").toInt());
    data.hpFormula = hpObj.value("formula").toString();


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

    data.speed = speed;

    /// Senses
    QJsonArray sensesObj = root["senses"].toArray();
    QString senses = "";

    for (const auto& senseVal : sensesObj) {
        senses += senseVal.toString();
    }
    data.senses = senses;


    /// Languages
    QJsonArray languagesList = root["languages"].toArray();
    QString languages = "";
    for (const auto& langVal: languagesList) {
        languages += langVal.toString();
    }
    data.languages = languages;

    /// Resistances and immunities
    QJsonArray damageImmunityList = root["immune"].toArray();
    QString damageImmunity = "";
    for (const auto& diVal : damageImmunityList) {
        damageImmunity += QString("%1, ").arg(diVal.toString());
    }
    data.damageImmunity = damageImmunity;

    QJsonArray damageResistanceList = root["resist"].toArray();
    QString damageResistance = "";
    for (const auto& drVal : damageResistanceList) {
        damageResistance += QString("%1, ").arg(drVal.toString());
    }
    data.damageResistance = damageResistance;

    QJsonArray conditionImmunityList = root["conditionImmune"].toArray();
    QString conditionImmunity = "";
    for (const auto& ciVal : conditionImmunityList) {
        conditionImmunity += QString("%1, ").arg(ciVal.toString());
    }
    data.conditionImmunity = conditionImmunity;

    QJsonArray damageVulnerabilityList = root["Vulnerability"].toArray();
    QString damageVulnerability = "";
    for (const auto& ciVal : damageVulnerabilityList) {
        damageVulnerability += QString("%1, ").arg(ciVal.toString());
    }
    data.damageVulnerability = damageVulnerability;


    /// Abilities
    QJsonObject savesObj = root["save"].toObject();

    data.abilities["str"] = root.value("str").toInt();
    data.abilities["dex"] = root.value("dex").toInt();
    data.abilities["con"] = root.value("con").toInt();
    data.abilities["int"] = root.value("int").toInt();
    data.abilities["wis"] = root.value("wis").toInt();
    data.abilities["cha"] = root.value("cha").toInt();


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
    data.skills = skillsHtml;


    /// Details
    QString race = root.value("type").toString();

    QJsonArray alignmentArray = root["alignment"].toArray();
    QString alignment= "";
    for (const auto& alignmentArrayVal: alignmentArray) {
        alignment += alignmentArrayVal.toString();
    }

    QString details = QString("%1, %2").arg(race, alignment);
    data.details = details;


    /// Danger
    data.dangerLevel = root.value("cr").toString();


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

    data.description = description;

    return data;
}

BestiaryPageData Fvtt11Parser::parse(const QJsonObject &root) {
    BestiaryPageData data;

    /// Name
   data.name = root.value("name").toString();

    QJsonObject systemObj = root["system"].toObject();

    QJsonObject attributesObj = systemObj["attributes"].toObject();


    /// Token
    data.imgLink = root.value("img").toString();


    /// AC
    QJsonObject acObj = attributesObj["ac"].toObject();
    data.ac = QString::number(acObj.value("flat").toInt());
    data.acCalc = acObj.value("calc").toString();


    /// HP
    QJsonObject hpObj = attributesObj["hp"].toObject();
    data.hp = QString::number(hpObj.value("max").toInt());
    data.hpFormula = hpObj.value("formula").toString();


    /// Proficiency
    data.prof = attributesObj.value("prof").toInt();


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

    data.speed = speed;

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

    data.senses = senses;


    /// Languages
    QJsonObject traitsObj = systemObj["traits"].toObject();
    QJsonArray languagesList = traitsObj["languages"].toObject()["value"].toArray();
    QString languages = "";
    for (const auto& langVal: languagesList) {
        languages += QString("%1, ").arg(langVal.toString());
    }
    languages += traitsObj["languages"].toObject().value("custom").toString();
    data.languages = languages;


    /// Resistances and immunities
    QJsonObject diObj = traitsObj["di"].toObject();
    QJsonArray damageImmunityList = diObj["value"].toArray();
    QString damageImmunity = "";
    for (const auto& diVal : damageImmunityList) {
        damageImmunity += QString("%1, ").arg(diVal.toString());
    }
    damageImmunity += diObj.value("custom").toString();
    data.damageImmunity = damageImmunity;

    QJsonObject drObj = traitsObj["dr"].toObject();
    QJsonArray damageResistanceList = drObj["value"].toArray();
    QString damageResistance = "";
    for (const auto& drVal : damageResistanceList) {
        damageResistance += QString("%1, ").arg(drVal.toString());
    }
    damageResistance += drObj.value("custom").toString();
    data.damageResistance = damageResistance;

    QJsonObject ciObj = traitsObj["ci"].toObject();
    QJsonArray conditionImmunityList = ciObj["value"].toArray();
    QString conditionImmunity = "";
    for (const auto& ciVal : conditionImmunityList) {
        conditionImmunity += QString("%1, ").arg(ciVal.toString());
    }
    conditionImmunity += ciObj.value("custom").toString();
    data.conditionImmunity = conditionImmunity;

    QJsonObject dvObj = traitsObj["dv"].toObject();
    QJsonArray damageVulnerabilityList = dvObj["value"].toArray();
    QString damageVulnerability = "";
    for (const auto& ciVal : damageVulnerabilityList) {
        damageVulnerability += QString("%1, ").arg(ciVal.toString());
    }
    damageVulnerability += dvObj.value("custom").toString();
    data.damageVulnerability = damageVulnerability;


    /// Abilities
    QJsonObject abilitiesObj = systemObj["abilities"].toObject();

    data.abilities["str"] = abilitiesObj["str"].toObject().value("value").toInt();
    data.abilityProf["str"] = (abilitiesObj["str"].toObject().value("proficient").toInt() > 0);

    data.abilities["dex"] = abilitiesObj["dex"].toObject().value("value").toInt();
    data.abilityProf["dex"] = (abilitiesObj["dex"].toObject().value("proficient").toInt() > 0);

    data.abilities["con"] = abilitiesObj["con"].toObject().value("value").toInt();
    data.abilityProf["con"] = (abilitiesObj["con"].toObject().value("proficient").toInt() > 0);

    data.abilities["int"] = abilitiesObj["int"].toObject().value("value").toInt();
    data.abilityProf["int"] = (abilitiesObj["int"].toObject().value("proficient").toInt() > 0);

    data.abilities["wis"] = abilitiesObj["wis"].toObject().value("value").toInt();
    data.abilityProf["wis"] = (abilitiesObj["wis"].toObject().value("proficient").toInt() > 0);

    data.abilities["cha"] = abilitiesObj["cha"].toObject().value("value").toInt();
    data.abilityProf["cha"] = (abilitiesObj["cha"].toObject().value("proficient").toInt() > 0);


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
            bonus = bonusFromStat(data.abilities["str"]) + data.prof;
        if (skillAbility == "dex")
            bonus = bonusFromStat(data.abilities["dex"]) + data.prof;
        if (skillAbility == "con")
            bonus = bonusFromStat(data.abilities["con"]) + data.prof;
        if (skillAbility == "int")
            bonus = bonusFromStat(data.abilities["int"]) + data.prof;
        if (skillAbility == "wis")
            bonus = bonusFromStat(data.abilities["wis"]) + data.prof;
        if (skillAbility == "cha")
            bonus = bonusFromStat(data.abilities["cha"]) + data.prof;

        skillBonus = (bonus > 0)? "+" + QString::number(bonus) : QString::number(bonus);

        skillsHtml += QString("%1: [[/r 1d20 %2]] (%2), ").arg(skillTitle, skillBonus);
    }
    data.skills = skillsHtml;


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
    data.details = details;

    data.dangerLevel = QString::number(detailsObj.value("cr").toInt());
    data.xp = QString::number(detailsObj["xp"].toObject().value("value").toInt());


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

    data.description = description;

    return data;
}