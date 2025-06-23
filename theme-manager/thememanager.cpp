#include "thememanager.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QDebug>

/**
 * @brief Applies the selected preset theme to the application.
 *
 * This function updates the application palette based on the specified
 * theme preset. It modifies various palette roles (like Window, WindowText,
 * Base, Text, Button, ButtonText, Highlight, and HighlightedText) to
 * correspond to the selected visual style. The new palette is then applied to
 * the application instance.
 *
 * @param theme The preset theme to apply. It can be either PresetTheme::Light
 * or PresetTheme::Dark. If an unknown value is provided, the default Light
 * theme is applied.
 */
void ThemeManager::applyPreset(ThemeManager::PresetTheme theme) {
    QPalette palette;
    switch (theme) {
        case PresetTheme::Light:
            palette.setColor(QPalette::Window, QColor("#f5f5f5"));
            palette.setColor(QPalette::WindowText, Qt::black);
            palette.setColor(QPalette::Base, Qt::white);
            palette.setColor(QPalette::Text, Qt::black);
            palette.setColor(QPalette::Button, QColor("#e0e0e0"));
            palette.setColor(QPalette::ButtonText, Qt::black);
            palette.setColor(QPalette::Highlight, QColor("#2196f3"));
            palette.setColor(QPalette::HighlightedText, Qt::white);
            break;
        case PresetTheme::Dark:
            palette.setColor(QPalette::Window, QColor("#121212"));
            palette.setColor(QPalette::WindowText, Qt::white);
            palette.setColor(QPalette::Base, QColor("#1e1e1e"));
            palette.setColor(QPalette::Text, QColor("#e0e0e0"));
            palette.setColor(QPalette::Button, QColor("#2d2d2d"));
            palette.setColor(QPalette::ButtonText, Qt::white);
            palette.setColor(QPalette::Highlight, QColor("#3f51b5"));
            palette.setColor(QPalette::HighlightedText, Qt::white);
            break;
        default:
            resetToSystemTheme();
            return;
    }
    qApp->setPalette(palette);
}

/**
 * Loads a theme configuration from an XML file and applies it to the application palette.
 *
 * The method parses an XML file containing theme configuration data,
 * including color roles and groups, and sets the corresponding colors
 * in the application palette. The XML structure is expected to contain
 * color groups (e.g., active, inactive, disabled) and color roles for
 * these groups (e.g., Window, Button, Text). The colors are defined
 * within the corresponding groups and roles.
 *
 * The method supports an extended color role "PlaceholderText" if the
 * application is built with Qt version 5.12 or higher.
 *
 * If the XML file cannot be loaded or parsed, a warning message is
 * logged, and the method returns false. Otherwise, the application's
 * palette is updated with the loaded theme, and the method returns true.
 *
 * @param path The file path to the XML theme configuration file.
 * @return Returns true if the theme is successfully loaded and applied,
 *         false if there is an error in loading or parsing the XML file.
 */
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

                if (xml.isStartElement() && xml.name() == QString("colorrole")) {
                    QString roleStr = xml.attributes().value("role").toString();

                    if (!roleMap.contains(roleStr))
                        continue;

                    QPalette::ColorRole role = roleMap[roleStr];

                    // Перейти к <color>
                    while (!(xml.isStartElement() && xml.name() == QString("color")) && !xml.atEnd())
                        xml.readNext();

                    if (xml.isStartElement() && xml.name() == QString("color")) {
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

void ThemeManager::resetToSystemTheme() {
    qApp->setPalette(QPalette());
}
