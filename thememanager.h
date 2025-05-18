//
// Created by arsen on 13.04.2025.
//

#ifndef DM_ASSIST_THEMEMANAGER_H
#define DM_ASSIST_THEMEMANAGER_H

#include <QPalette>
#include <QShortcut>
#include <QMap>
#include <QColor>
#include <QApplication>

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
        Light,
        Dark
    };

    static void applyPreset(PresetTheme theme);
    static bool loadFromXml(const QString& path);
};


#endif //DM_ASSIST_THEMEMANAGER_H
