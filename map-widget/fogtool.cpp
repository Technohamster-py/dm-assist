//
// Created by arseniy on 03.05.2025.
//

#include "fogtool.h"
#include "mapscene.h"

FogTool::FogTool(QObject *parent) : AbstractMapTool(parent) {}

void FogTool::setMode(FogTool::Mode mode) {
    currentMode = mode;
}

void FogTool::setRadius(int radius) {
    brushRadius = radius;
}

void FogTool::mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    auto mapScene = dynamic_cast<MapScene*>(scene);
    if (mapScene) {
        mapScene->drawFogCircle(event->scenePos(), brushRadius, currentMode == Hide);
    }
}

void FogTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    auto mapScene = dynamic_cast<MapScene*>(scene);
    if (mapScene && (event->buttons() & Qt::LeftButton)) {
        mapScene->drawFogCircle(event->scenePos(), brushRadius, currentMode == Hide);
    }
}
