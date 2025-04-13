//
// Created by arsen on 13.04.2025.
//

#include "thememanager.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QDebug>

void ThemeManager::applyPreset(ThemeManager::PresetTheme theme) {
    QPalette palette;
    if (theme == PresetTheme::Light){
        palette.setColor(QPalette::Window, QColor("#f5f5f5"));
        palette.setColor(QPalette::WindowText, Qt::black);
        palette.setColor(QPalette::Base, Qt::white);
        palette.setColor(QPalette::Text, Qt::black);
        palette.setColor(QPalette::Button, QColor("#e0e0e0"));
        palette.setColor(QPalette::ButtonText, Qt::black);
        palette.setColor(QPalette::Highlight, QColor("#2196f3"));
        palette.setColor(QPalette::HighlightedText, Qt::white);
    } else {
        palette.setColor(QPalette::Window, QColor("#121212"));
        palette.setColor(QPalette::WindowText, Qt::white);
        palette.setColor(QPalette::Base, QColor("#1e1e1e"));
        palette.setColor(QPalette::Text, QColor("#e0e0e0"));
        palette.setColor(QPalette::Button, QColor("#2d2d2d"));
        palette.setColor(QPalette::ButtonText, Qt::white);
        palette.setColor(QPalette::Highlight, QColor("#3f51b5"));
        palette.setColor(QPalette::HighlightedText, Qt::white);
    }
    qApp->setPalette(palette);
}

bool ThemeManager::loadFromXml(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open theme file:" << path;
        return false;
    }

    QXmlStreamReader xml(&file);
    QPalette palette;

    QMap<QString, QPalette::ColorRole> roleMap = {
            {"Window", QPalette::Window},
            {"WindowText", QPalette::WindowText},
            {"Base", QPalette::Base},
            {"AlternateBase", QPalette::AlternateBase},
            {"ToolTipBase", QPalette::ToolTipBase},
            {"ToolTipText", QPalette::ToolTipText},
            {"Text", QPalette::Text},
            {"Button", QPalette::Button},
            {"ButtonText", QPalette::ButtonText},
            {"BrightText", QPalette::BrightText},
            {"Highlight", QPalette::Highlight},
            {"HighlightedText", QPalette::HighlightedText},
            {"Link", QPalette::Link},
            {"LinkVisited", QPalette::LinkVisited},
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
            {"PlaceholderText", QPalette::PlaceholderText},
#endif
    };

    QMap<QString, QPalette::ColorGroup> groupMap = {
            {"active", QPalette::Active},
            {"inactive", QPalette::Inactive},
            {"disabled", QPalette::Disabled}
    };

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement() && groupMap.contains(xml.name().toString().toLower())) {
            QPalette::ColorGroup group = groupMap[xml.name().toString().toLower()];

            while (!(xml.isEndElement() && xml.name().toString().toLower() == groupMap.key(group)) && !xml.atEnd()) {
                xml.readNext();

                if (xml.isStartElement() && xml.name() == "colorrole") {
                    QString roleStr = xml.attributes().value("role").toString();

                    if (!roleMap.contains(roleStr))
                        continue;

                    QPalette::ColorRole role = roleMap[roleStr];

                    // Перейти к <color>
                    while (!(xml.isStartElement() && xml.name() == "color") && !xml.atEnd())
                        xml.readNext();

                    if (xml.isStartElement() && xml.name() == "color") {
                        auto a = xml.attributes();
                        QColor color(
                                a.value("red").toInt(),
                                a.value("green").toInt(),
                                a.value("blue").toInt(),
                                a.hasAttribute("alpha") ? a.value("alpha").toInt() : 255
                        );
                        palette.setColor(group, role, color);
                    }
                }
            }
        }
    }

    if (xml.hasError()) {
        qWarning() << "XML parse error:" << xml.errorString();
        return false;
    }

    qApp->setPalette(palette);
    return true;
}
