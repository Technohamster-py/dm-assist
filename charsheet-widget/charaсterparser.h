#ifndef DM_ASSIST_CHARAСTERPARSER_H
#define DM_ASSIST_CHARAСTERPARSER_H

#include <QVariant>
#include <QMap>
#include <QFileInfo>

#include "dndCharacterData.h"

class ICharacterParser {
public:
    enum filetypes {
        json,
        xml,
        none
    };
    virtual ~ICharacterParser() = default;
    virtual QVariantMap parse(const QString& filePath) = 0;
    virtual bool writeToFile(const QVariantMap& dataMap, const QString& filePath = ""){return true;}
    static filetypes fileExt(const QString& filePath) {
        QFileInfo fi(filePath);
        if (!fi.exists()) return none;

        QString ext = fi.suffix();
        if (ext == "json") return json;
        if (ext == "xml") return xml;

        return none;
    }
};


class IDndParser : public ICharacterParser {
public:
    virtual DndCharacterData parseDnd(const QString& filePath) = 0;
    virtual bool writeDnd(const DndCharacterData& data, const QString& filePath) {return true;}
};


class LssDndParser : public IDndParser {
public:
    QVariantMap parse(const QString& filePath) override;
    bool writeDnd(const DndCharacterData& data, const QString& filePath) override;
    DndCharacterData parseDnd(const QString& filePath) override;
    virtual QString parseParagraphs(const QJsonArray &content);
    static QJsonArray serializeHtmlToJson(const QString &html);
};


#endif //DM_ASSIST_CHARAСTERPARSER_H
