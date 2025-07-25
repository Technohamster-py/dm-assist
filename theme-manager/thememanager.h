#ifndef DM_ASSIST_THEMEMANAGER_H
#define DM_ASSIST_THEMEMANAGER_H

#include <QPalette>
#include <QShortcut>
#include <QMap>
#include <QColor>
#include <QApplication>


static QMap<QString, QPalette::ColorRole> roleMap = {
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

static QMap<QString, QPalette::ColorGroup> groupMap = {
        {"active", QPalette::Active},
        {"inactive", QPalette::Inactive},
        {"disabled", QPalette::Disabled}
};

/**
 * @class ThemeManager
 * @brief Manages application themes through preset options or custom XML-based themes.
 *
 * The ThemeManager class allows users to apply predefined "Light" or "Dark" themes
 * and also enables dynamic theme loading from custom XML files to customize the application's appearance.
 */
class ThemeManager{
public:
    enum class PresetTheme{
        System,
        Light,
        Dark
    };

    static void applyPreset(PresetTheme theme);
    static bool loadFromXml(const QString& path);
    static void resetToSystemTheme();
};


#endif //DM_ASSIST_THEMEMANAGER_H
