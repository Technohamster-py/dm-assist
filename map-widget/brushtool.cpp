#include "brushtool.h"
#include "mapscene.h"
#include <QGraphicsSceneMouseEvent>
#include <QPen>

BrushTool::BrushTool(QObject* parent) : AbstractMapTool(parent) {}

void BrushTool::setColor(const QColor& c) {
    color = c;
}

void BrushTool::setOpacity(qreal a) {
    opacity = qBound(0.0, a, 1.0);
}

void BrushTool::setBrushSize(int px) {
    brushSize = px;
}

void BrushTool::mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    MapScene* mapScene = dynamic_cast<MapScene*>(scene);
    if (event->button() == Qt::RightButton) {
        QGraphicsItem* clicked = mapScene->itemAt(event->scenePos(), QTransform());
        if (auto* path = qgraphicsitem_cast<QGraphicsPathItem*>(clicked)) {
            if (path->zValue() == 6) {
                mapScene->removeUndoableItem(path);
                return;
            }
        }
        return;
    }

    path = QPainterPath();
    path.moveTo(event->scenePos());

    previewItem = new QGraphicsPathItem();
    QPen pen(color, brushSize, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    pen.setCosmetic(true);
    previewItem->setPen(pen);
    previewItem->setOpacity(opacity);
    previewItem->setZValue(mapLayers::Brush);
    previewItem->setPath(path);

    mapScene->addItem(previewItem);
}

void BrushTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (previewItem) {
        path.lineTo(event->scenePos());
        previewItem->setPath(path);
    }
}

void BrushTool::mouseReleaseEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (previewItem) {
        path.lineTo(event->scenePos());
        previewItem->setPath(path);

        previewItem->setParentItem(nullptr);
        static_cast<MapScene*>(scene)->addUndoableItem(previewItem);
        previewItem = nullptr;
    }
}

void BrushTool::deactivate(QGraphicsScene *scene) {
    if (previewItem) {
        previewItem->scene()->removeItem(previewItem);
        delete previewItem;
        previewItem = nullptr;
    }
    path = QPainterPath();
}

void BrushTool::wheelEvent(QGraphicsSceneWheelEvent *event, QGraphicsScene *scene) {
    int delta = event->delta() > 0 ? 5 : -5;
    brushSize = std::clamp(brushSize + delta, 5, 300);
    event->accept();
}

void BrushTool::clearAll(QGraphicsScene *scene) {
    for (QGraphicsItem* item : scene->items()) {
        if (auto* path = qgraphicsitem_cast<QGraphicsPathItem*>(item)) {
            if (path->zValue() == 6) {
                scene->removeItem(path);
                delete path;
            }
        }
    }
}

