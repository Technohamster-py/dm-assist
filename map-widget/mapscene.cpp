/**
 * @file mapscene.cpp
 * @brief Implementation of MapScene class.
 */

#include "mapscene.h"

/**
 * @brief Constructs a new MapScene.
 * @param parent Optional parent QObject
 */
MapScene::MapScene(QObject *parent)
        : QGraphicsScene(parent)
{
    // Подготовка сцены. Инструменты и слои будут добавлены позже.
    setBackgroundBrush(Qt::darkGray);
}
