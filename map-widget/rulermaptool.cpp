//
// Created by arseniy on 02.05.2025.
//

#include "rulermaptool.h"
#include "mapscene.h"

RulerMapTool::RulerMapTool(QObject *parent) : AbstractMapTool(parent) {}

void RulerMapTool::mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    QPointF point = event->scenePos();

    if (event->button() == Qt::RightButton) {
        QGraphicsItem *item = scene->itemAt(point, QTransform());
        if (permanentItems.contains(item)) {
            scene->removeItem(item);
            permanentItems.removeOne(item);
            delete item;
        }
        return;
    }

    if (toolPoints.isEmpty()) {
        toolPoints.append(point);
    } else {
        toolPoints.append(point);
        QLineF line(toolPoints[0], toolPoints[1]);
        if (auto mapScene = qobject_cast<MapScene*>(scene)) {
            double distance = line.length() * mapScene->getScaleFactor();
            auto lineItem = scene->addLine(line, QPen(Qt::red, 2));
            auto label = scene->addText(QString("%1 ft").arg(distance, 0, 'f', 1));
            label->setDefaultTextColor(Qt::red);
            label->setPos((line.p1() + line.p2()) / 2);

            lineItem->setZValue(10);
            label->setZValue(11);

            permanentItems.append(lineItem);
            permanentItems.append(label);

            if (tempLine) scene->removeItem(tempLine);
            if (tempLabel) scene->removeItem(tempLabel);
            delete tempLine;
            delete tempLabel;
            tempLine = nullptr;
            tempLabel = nullptr;

            toolPoints.clear();
        }
    }
}

void RulerMapTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (toolPoints.size() == 1) {
        QPointF current = event->scenePos();
        QLineF line(toolPoints[0], current);
        if (auto mapScene = qobject_cast<MapScene*>(scene)) {
            double distance = line.length() * mapScene->getScaleFactor();

            if (!tempLine) {
                tempLine = scene->addLine(line, QPen(Qt::DashLine));
            } else {
                tempLine->setLine(line);
            }

            if (!tempLabel) {
                tempLabel = scene->addText(QString("%1 ft").arg(distance, 0, 'f', 1));
                tempLabel->setDefaultTextColor(Qt::gray);
            } else {
                tempLabel->setPlainText(QString("%1 ft").arg(distance, 0, 'f', 1));
            }
            tempLabel->setPos((line.p1() + line.p2()) / 2);
        }
    }
}

void RulerMapTool::deactivate(QGraphicsScene *scene) {
    toolPoints.clear();

    if (tempLine) {
        scene->removeItem(tempLine);
        delete tempLine;
        tempLine = nullptr;
    }
    if (tempLabel) {
        scene->removeItem(tempLabel);
        delete tempLabel;
        tempLabel = nullptr;
    }
}
