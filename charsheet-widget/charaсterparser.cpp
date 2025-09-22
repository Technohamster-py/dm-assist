#include "charaсterparser.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextList>

QVariantMap LssDndParser::parse(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return {};
    QVariantMap result;

    QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = document.object();
    file.close();

    QString dataString = root.value("data").toString();
    QJsonObject m_dataObject = QJsonDocument::fromJson(dataString.toUtf8()).object();

    result["name"] = m_dataObject["name"].toObject()["value"].toString();

    QJsonObject infoObject = m_dataObject["info"].toObject();
    QString classString = QString("%1 (%2)").arg(infoObject["charClass"].toObject()["value"].toString(), infoObject["charSubclass"].toObject()["value"].toString());
    result["class"] = classString;
    result["level"] = infoObject["level"].toObject()["value"].toInt();

    QJsonObject vitalityObject = m_dataObject["vitality"].toObject();
    result["speed"] = vitalityObject["speed"].toObject()["value"].toString().toInt();
    result["ac"] = vitalityObject["ac"].toObject()["value"].toInt();
    result["hp"] = vitalityObject["hp-current"].toObject()["value"].toInt();
    result["maxHp"] = vitalityObject["hp-max"].toObject()["value"].toInt();

    QJsonObject statsObject = m_dataObject["stats"].toObject();
    QJsonObject savesObject = m_dataObject["saves"].toObject();

    result["strValue"] = statsObject["str"].toObject()["score"].toInt();
    result["dexValue"] = statsObject["dex"].toObject()["score"].toInt();
    result["conValue"] = statsObject["con"].toObject()["score"].toInt();
    result["intValue"] = statsObject["int"].toObject()["score"].toInt();
    result["wisValue"] = statsObject["wis"].toObject()["score"].toInt();
    result["chaValue"] = statsObject["cha"].toObject()["score"].toInt();

    result["strProf"] = savesObject["str"].toObject()["isProf"].toBool();
    result["dexProf"] = savesObject["dex"].toObject()["isProf"].toBool();
    result["conProf"] = savesObject["con"].toObject()["isProf"].toBool();
    result["intProf"] = savesObject["int"].toObject()["isProf"].toBool();
    result["wisProf"] = savesObject["wis"].toObject()["isProf"].toBool();
    result["chaProf"] = savesObject["cha"].toObject()["isProf"].toBool();

    QJsonObject skillsObject = m_dataObject["skills"].toObject();
    result["athletics"] = skillsObject["athletics"].toObject()["isProf"].toInt();
    result["acrobatics"] = skillsObject["acrobatics"].toObject()["isProf"].toInt();
    result["sleight of hand"] = skillsObject["sleight of hand"].toObject()["isProf"].toInt();
    result["stealth"] = skillsObject["stealth"].toObject()["isProf"].toInt();
    result["arcana"] = skillsObject["arcana"].toObject()["isProf"].toInt();
    result["history"] = skillsObject["history"].toObject()["isProf"].toInt();
    result["investigation"] = skillsObject["investigation"].toObject()["isProf"].toInt();
    result["nature"] = skillsObject["nature"].toObject()["isProf"].toInt();
    result["religion"] = skillsObject["religion"].toObject()["isProf"].toInt();
    result["animal handling"] = skillsObject["animal handling"].toObject()["isProf"].toInt();
    result["insight"] = skillsObject["insight"].toObject()["isProf"].toInt();
    result["medicine"] = skillsObject["medicine"].toObject()["isProf"].toInt();
    result["perception"] = skillsObject["perception"].toObject()["isProf"].toInt();
    result["survival"] = skillsObject["survival"].toObject()["isProf"].toInt();
    result["deception"] = skillsObject["deception"].toObject()["isProf"].toInt();
    result["intimidation"] = skillsObject["intimidation"].toObject()["isProf"].toInt();
    result["performance"] = skillsObject["performance"].toObject()["isProf"].toInt();
    result["persuasion"] = skillsObject["persuasion"].toObject()["isProf"].toInt();

    result["proficiencies"] = parseParagraphs(m_dataObject["text"].toObject()["prof"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result["traits"] = parseParagraphs(m_dataObject["text"].toObject()["traits"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result["equipment"] = parseParagraphs(m_dataObject["text"].toObject()["equipment"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result["featuresEdit"] = parseParagraphs(m_dataObject["text"].toObject()["features"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result["allies"] = parseParagraphs(m_dataObject["text"].toObject()["allies"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result["personality"] = parseParagraphs(m_dataObject["text"].toObject()["personality"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result["background"] = parseParagraphs(m_dataObject["text"].toObject()["background"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result["quests"] = parseParagraphs(m_dataObject["text"].toObject()["quests"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result["ideals"] = parseParagraphs(m_dataObject["text"].toObject()["ideals"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result["bonds"] = parseParagraphs(m_dataObject["text"].toObject()["bonds"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result["flaws"] = parseParagraphs(m_dataObject["text"].toObject()["flaws"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());

    QString notes;
    notes += parseParagraphs(m_dataObject["text"].toObject()["notes-1"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    notes += parseParagraphs(m_dataObject["text"].toObject()["notes-2"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    notes += parseParagraphs(m_dataObject["text"].toObject()["notes-3"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    notes += parseParagraphs(m_dataObject["text"].toObject()["notes-4"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    notes += parseParagraphs(m_dataObject["text"].toObject()["notes-5"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    notes += parseParagraphs(m_dataObject["text"].toObject()["notes-6"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result["notes"] = notes;

    result["weaponList"] = m_dataObject["weaponsList"].toArray();
    result["resourcedList"] = m_dataObject["resources"].toVariant();

    return result;
}

QString LssDndParser::parseParagraphs(const QJsonArray &content) {
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

DndCharacterData LssDndParser::parseDnd(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return {};
    DndCharacterData result;

    QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = document.object();
    file.close();

    QString dataString = root.value("data").toString();
    QJsonObject dataObject = QJsonDocument::fromJson(dataString.toUtf8()).object();

    result.name = dataObject["name"].toObject()["value"].toString();

    QJsonObject infoObject = dataObject["info"].toObject();
    QString classString = QString("%1 (%2)").arg(infoObject["charClass"].toObject()["value"].toString(), infoObject["charSubclass"].toObject()["value"].toString());
    result.className = classString;
    result.level = infoObject["level"].toObject()["value"].toInt();

    QJsonObject vitalityObject = dataObject["vitality"].toObject();
    result.speed = vitalityObject["speed"].toObject()["value"].toString().toInt();
    result.ac = vitalityObject["ac"].toObject()["value"].toInt();
    result.hp = vitalityObject["hp-current"].toObject()["value"].toInt();
    result.maxHp = vitalityObject["hp-max"].toObject()["value"].toInt();

    QJsonObject statsObject = dataObject["stats"].toObject();
    QJsonObject savesObject = dataObject["saves"].toObject();
    QJsonObject skillsObject = dataObject["skills"].toObject();

    result.stats["str"] = statsObject["str"].toObject()["score"].toInt();
    result.stats["dex"] = statsObject["dex"].toObject()["score"].toInt();
    result.stats["con"] = statsObject["con"].toObject()["score"].toInt();
    result.stats["int"] = statsObject["int"].toObject()["score"].toInt();
    result.stats["wis"] = statsObject["wis"].toObject()["score"].toInt();
    result.stats["cha"] = statsObject["cha"].toObject()["score"].toInt();

    result.statProf["str"] = savesObject["str"].toObject()["isProf"].toBool();
    result.statProf["dex"] = savesObject["dex"].toObject()["isProf"].toBool();
    result.statProf["con"] = savesObject["con"].toObject()["isProf"].toBool();
    result.statProf["int"] = savesObject["int"].toObject()["isProf"].toBool();
    result.statProf["wis"] = savesObject["wis"].toObject()["isProf"].toBool();
    result.statProf["cha"] = savesObject["cha"].toObject()["isProf"].toBool();

    result.skillsProf["athletics"] = skillsObject["athletics"].toObject()["isProf"].toInt() > 0;
    result.skillsProf["acrobatics"] = skillsObject["acrobatics"].toObject()["isProf"].toInt() > 0;
    result.skillsProf["sleight of hand"] = skillsObject["sleight of hand"].toObject()["isProf"].toInt() > 0;
    result.skillsProf["stealth"] = skillsObject["stealth"].toObject()["isProf"].toInt() > 0;
    result.skillsProf["arcana"] = skillsObject["arcana"].toObject()["isProf"].toInt() > 0;
    result.skillsProf["history"] = skillsObject["history"].toObject()["isProf"].toInt() > 0;
    result.skillsProf["investigation"] = skillsObject["investigation"].toObject()["isProf"].toInt() > 0;
    result.skillsProf["nature"] = skillsObject["nature"].toObject()["isProf"].toInt() > 0;
    result.skillsProf["religion"] = skillsObject["religion"].toObject()["isProf"].toInt() > 0;
    result.skillsProf["animal handling"] = skillsObject["animal handling"].toObject()["isProf"].toInt() > 0;
    result.skillsProf["insight"] = skillsObject["insight"].toObject()["isProf"].toInt() > 0;
    result.skillsProf["medicine"] = skillsObject["medicine"].toObject()["isProf"].toInt() > 0;
    result.skillsProf["perception"] = skillsObject["perception"].toObject()["isProf"].toInt() > 0;
    result.skillsProf["survival"] = skillsObject["survival"].toObject()["isProf"].toInt() > 0;
    result.skillsProf["deception"] = skillsObject["deception"].toObject()["isProf"].toInt() > 0;
    result.skillsProf["intimidation"] = skillsObject["intimidation"].toObject()["isProf"].toInt() > 0;
    result.skillsProf["performance"] = skillsObject["performance"].toObject()["isProf"].toInt() > 0;
    result.skillsProf["persuasion"] = skillsObject["persuasion"].toObject()["isProf"].toInt() > 0;

    result.proficienciesHtml = parseParagraphs(dataObject["text"].toObject()["prof"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result.traitsHtml = parseParagraphs(dataObject["text"].toObject()["traits"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result.equipmentHtml = parseParagraphs(dataObject["text"].toObject()["equipment"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result.featuresHtml = parseParagraphs(dataObject["text"].toObject()["features"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result.alliesHtml = parseParagraphs(dataObject["text"].toObject()["allies"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result.personalityHtml = parseParagraphs(dataObject["text"].toObject()["personality"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result.backgroundHtml = parseParagraphs(dataObject["text"].toObject()["background"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result.questsHtml = parseParagraphs(dataObject["text"].toObject()["quests"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result.idealsHtml = parseParagraphs(dataObject["text"].toObject()["ideals"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result.bondsHtml = parseParagraphs(dataObject["text"].toObject()["bonds"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result.flawsHtml = parseParagraphs(dataObject["text"].toObject()["flaws"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());

    QString notes;
    notes += parseParagraphs(dataObject["text"].toObject()["notes-1"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    notes += parseParagraphs(dataObject["text"].toObject()["notes-2"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    notes += parseParagraphs(dataObject["text"].toObject()["notes-3"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    notes += parseParagraphs(dataObject["text"].toObject()["notes-4"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    notes += parseParagraphs(dataObject["text"].toObject()["notes-5"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    notes += parseParagraphs(dataObject["text"].toObject()["notes-6"].toObject()["value"].toObject()["data"].toObject()["content"].toArray());
    result.notes = notes;

    result.weapons = dataObject["weaponsList"].toArray();
    result.resourcesObj = dataObject["resources"].toObject();

    return result;
}

bool LssDndParser::writeDnd(const DndCharacterData &data, const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = document.object();
    file.close();
    QString dataString = root.value("data").toString();
    QJsonObject dataObj = QJsonDocument::fromJson(dataString.toUtf8()).object();

    dataObj["name"] = QJsonObject{
            {"value", data.name}
    };

    // info
    QJsonObject info;
    info["charClass"] = QJsonObject{{"value", data.className}};
    info["charSubclass"] = QJsonObject{{"value", data.subclass}};
    info["level"] = QJsonObject{{"value", data.level}};
    info["playerName"] = QJsonObject{{"value", ""}};
    info["background"] = QJsonObject{{"value", dataObj["info"].toObject().value("background").toObject().value("value")}};
    info["race"] = QJsonObject{{"value", dataObj["info"].toObject().value("race").toObject().value("value")}};
    info["alignment"] = QJsonObject{{"value", dataObj["info"].toObject().value("alignment").toObject().value("value")}};
    info["experience"] = QJsonObject{{"value", ""}};
    dataObj["info"] = info;

    // vitality
    QJsonObject vitality;
    vitality["speed"] = QJsonObject{{"value", data.speed}};
    vitality["ac"] = QJsonObject{{"value", data.ac}};
    vitality["hp-current"] = QJsonObject{{"value", data.hp}};
    vitality["hp-max"] = QJsonObject{{"value", data.maxHp}};
    dataObj["vitality"] = vitality;

    // stats
    auto statObject = [&](const QString& statName, int score) {
        return QJsonObject{{"score", score}, {"modifier", (score >= 10) ? (score - 10) / 2 : (score - 11) / 2}};
    };

    QJsonObject stats;
    stats["str"] = statObject("str", data.stats["str"]);
    stats["dex"] = statObject("dex", data.stats["dex"]);
    stats["con"] = statObject("con", data.stats["con"]);
    stats["int"] = statObject("int", data.stats["int"]);
    stats["wis"] = statObject("wis", data.stats["wis"]);
    stats["cha"] = statObject("cha", data.stats["cha"]);
    dataObj["stats"] = stats;

    // saves
    QJsonObject saves;
    saves["str"] = QJsonObject{{"isProf", data.statProf["str"]}};
    saves["dex"] = QJsonObject{{"isProf", data.statProf["dex"]}};
    saves["con"] = QJsonObject{{"isProf", data.statProf["con"]}};
    saves["int"] = QJsonObject{{"isProf", data.statProf["int"]}};
    saves["wis"] = QJsonObject{{"isProf", data.statProf["wis"]}};
    saves["cha"] = QJsonObject{{"isProf", data.statProf["cha"]}};
    dataObj["saves"] = saves;

    // skills
    auto skill = [&](const QString& name, bool isProf, const QString& stat = "") {
        QJsonObject obj;
        obj["name"] = name;
        if (!stat.isEmpty()) obj["baseStat"] = stat;
        if (isProf) obj["isProf"] = 1;
        return obj;
    };

    QJsonObject skills;
    skills["acrobatics"] = skill("acrobatics", data.skillsProf["acrobatics"], "dex");
    skills["athletics"] = skill("athletics", data.skillsProf["athletics"], "str");
    skills["sleight of hand"] = skill("sleight of hand", data.skillsProf["sleight of hand"], "dex");
    skills["stealth"] = skill("stealth", data.skillsProf["stealth"], "dex");
    skills["arcana"] = skill("arcana", data.skillsProf["arcana"], "int");
    skills["history"] = skill("history", data.skillsProf["history"], "int");
    skills["investigation"] = skill("investigation", data.skillsProf["investigation"], "int");
    skills["nature"] = skill("nature", data.skillsProf["nature"], "int");
    skills["religion"] = skill("religion", data.skillsProf["religion"], "int");
    skills["animal handling"] = skill("animal handling", data.skillsProf["animal handling"], "wis");
    skills["insight"] = skill("insight", data.skillsProf["insight"], "wis");
    skills["medicine"] = skill("medicine", data.skillsProf["medicine"], "wis");
    skills["perception"] = skill("perception", data.skillsProf["perception"], "wis");
    skills["survival"] = skill("survival", data.skillsProf["survival"], "wis");
    skills["deception"] = skill("deception", data.skillsProf["deception"], "cha");
    skills["intimidation"] = skill("intimidation", data.skillsProf["intimidation"], "cha");
    skills["performance"] = skill("performance", data.skillsProf["performance"], "cha");
    skills["persuasion"] = skill("persuasion", data.skillsProf["persuasion"], "cha");
    dataObj["skills"] = skills;

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
    text["prof"] = toDoc(data.proficienciesHtml);
    text["traits"] = toDoc(data.traitsHtml);
    text["equipment"] = toDoc(data.equipmentHtml);
    text["features"] = toDoc(data.featuresHtml);
    text["allies"] = toDoc(data.alliesHtml);
    text["personality"] = toDoc(data.personalityHtml);
    text["background"] = toDoc(data.backgroundHtml);
    text["quests"] = toDoc(data.questsHtml);
    text["ideals"] = toDoc(data.idealsHtml);
    text["bonds"] = toDoc(data.bondsHtml);
    text["flaws"] = toDoc(data.flawsHtml);
    dataObj["text"] = text;

    // оружие и ресурсы
    dataObj["weaponsList"] = data.weapons;
    dataObj["resources"] = data.resourcesObj;


    QJsonDocument innerDoc(dataObj);
    QString jsonString = QString::fromUtf8(innerDoc.toJson(QJsonDocument::Compact));

    root["data"] = jsonString;

    document = QJsonDocument(root);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) return false;
    file.write(document.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

QJsonArray LssDndParser::serializeHtmlToJson(const QString &html) {
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
