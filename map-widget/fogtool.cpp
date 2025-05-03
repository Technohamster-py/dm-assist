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
    updateBrushPreview(event->scenePos(), scene);

    auto mapScene = dynamic_cast<MapScene*>(scene);
    if (mapScene) {
        mapScene->drawFogCircle(event->scenePos(), brushRadius, currentMode == Hide);
    }
}

void FogTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    updateBrushPreview(event->scenePos(), scene);

    auto mapScene = dynamic_cast<MapScene*>(scene);
    if (mapScene && (event->buttons() & Qt::LeftButton)) {
        mapScene->drawFogCircle(event->scenePos(), brushRadius, currentMode == Hide);
    }
}

void FogTool::updateBrushPreview(const QPointF &scenePos, QGraphicsScene *scene) {
    if (!scene) return;

    QPointF topLeft = scenePos - QPointF(brushRadius, brushRadius);
    QSizeF size(brushRadius * 2, brushRadius * 2);

    if (!brushPreview) {
        brushPreview = scene->addEllipse(QRectF(topLeft, size),
                                         QPen(QBrush(Qt::DashLine), 1.5, Qt::DashLine),
                                         Qt::NoBrush);
        brushPreview->setZValue(100); // выше всего
    } else {
        brushPreview->setRect(QRectF(topLeft, size));
    }
}

void FogTool::removeBrushPreview(QGraphicsScene *scene) {
    if (brushPreview && scene) {
        scene->removeItem(brushPreview);
        delete brushPreview;
        brushPreview = nullptr;
    }
}

void FogTool::deactivate(QGraphicsScene *scene) {
    removeBrushPreview(scene);
}

void FogTool::wheelEvent(QGraphicsSceneWheelEvent *event, QGraphicsScene *scene) {
    int delta = event->delta() > 0 ? 5 : -5;
    brushRadius = std::clamp(brushRadius + delta, 5, 300);
    updateBrushPreview(event->scenePos(), scene);
    event->accept();
}
