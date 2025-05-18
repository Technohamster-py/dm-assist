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
