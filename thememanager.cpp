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

bool ThemeManager::loadFromXml(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }
}
