/**
 * @file mapscene.cpp
 * @brief Implementation of MapScene class.
 */

#include "mapscene.h"
#include <QInputDialog>
#include <QMessageBox>

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

void MapScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (m_activeTool)
        m_activeTool->mousePressEvent(event, this);
    else
        QGraphicsScene::mousePressEvent(event);
}

void MapScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (m_activeTool)
        m_activeTool->mouseMoveEvent(event, this);
    else
        QGraphicsScene::mouseMoveEvent(event);
}

void MapScene::setScaleFactor(double factor) {
    m_scaleFactor = factor;
}

void MapScene::setActiveTool(AbstractMapTool *tool) {
    if (m_activeTool)
        m_activeTool->deactivate(this);
    m_activeTool = tool;
}
