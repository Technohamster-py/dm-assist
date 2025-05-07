#include "spellshapetool.h"
#include "mapscene.h"

SpellShapeTool::SpellShapeTool(QObject *parent)
        : AbstractMapTool(parent) {
    color.setAlpha(180);
}

void SpellShapeTool::clearPreview(QGraphicsScene *scene) {
    if (previewShape) {
        scene->removeItem(previewShape);
        delete previewShape;
        previewShape = nullptr;
    }
    if (previewLabel) {
        scene->removeItem(previewLabel);
        delete previewLabel;
        previewLabel = nullptr;
    }
}

void SpellShapeTool::setColor(QColor c) {
    color = c;
    color.setAlpha(180);
}

bool SpellShapeTool::clearShapeAt(QGraphicsScene *scene, QPointF point) {
    return true;
}



void LineShapeTool::mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (!hasFirstPoint){
        if (event->button() == Qt::RightButton) {
            clearShapeAt(scene, event->scenePos());
            return;
        }

        firstPoint = event->scenePos();
        hasFirstPoint = true;
    } else {
        if (event->button() == Qt::RightButton){
            hasFirstPoint = false;
            clearPreview(scene);
            return;
        }

        QLineF line(firstPoint, event->scenePos());
        scene->addLine(line, QPen(color, 2));
        hasFirstPoint = false;
        clearPreview(scene);
    }
}

void LineShapeTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (!hasFirstPoint)
        return;

    QLineF line(firstPoint, event->scenePos());
    if (!previewShape) {
        previewShape = scene->addLine(line, QPen(Qt::DashLine));
    } else {
        static_cast<QGraphicsLineItem*>(previewShape)->setLine(line);
    }

    double distance = 0.0;

    if (auto mapScene = qobject_cast<MapScene*>(scene)){
        distance = line.length() * mapScene->getScaleFactor();
    } else {
        distance = line.length();
    }

    if (!previewLabel) {
        previewLabel = scene->addText(QString("%1 ft").arg(distance, 0, 'f', 1));
    } else {
        previewLabel->setPlainText(QString("%1 ft").arg(distance, 0, 'f', 1));
    }
    previewLabel->setPos(line.pointAt(0.5) + QPointF(10, -10));
}
