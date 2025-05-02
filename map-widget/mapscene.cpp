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

void MapScene::setToolMode(ToolMode mode) {
    m_toolMode = mode;
    toolPoints.clear();
}

void MapScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    QPointF clickedPoint = event->scenePos();

    if (event->button() == Qt::RightButton){
        QGraphicsItem* item = itemAt(clickedPoint, QTransform());
        if (measurementItems.contains(item)){
            removeItem(item);
            measurementItems.removeOne(item);
            if (item->type() == QGraphicsLineItem::Type || item->type() == QGraphicsTextItem::Type){
                delete item;
            }
        }
        return;
    }

    if (m_toolMode == ToolMode::Calibration){
        toolPoints.append(clickedPoint);

        if (toolPoints.size() == 2){
            double pixelDistance = QLineF(toolPoints[0], toolPoints[1]).length();
            bool ok;
            double realDistance = QInputDialog::getDouble(nullptr,
                                                          tr("Scale calibration"),
                                                          tr("Enter the distance in feet"),
                                                          5.0,
                                                          0.1,
                                                          2000,
                                                          2,
                                                          &ok);
            if (ok && pixelDistance != 0.0)
                scaleFactor = realDistance/pixelDistance;
            setToolMode(ToolMode::None);
        }
    } else if (m_toolMode == ToolMode::Measure){
        toolPoints.append(clickedPoint);
        if (toolPoints.size() == 2){
            double pixelDistance = QLineF(toolPoints[0], toolPoints[1]).length();
            double realDistance = pixelDistance * scaleFactor;

            auto lineItem = addLine(QLineF(toolPoints[0], toolPoints[1]), QPen(Qt::red, 2));
            lineItem->setZValue(9);
            measurementItems.append(lineItem);
            QString distanceLabel = QString("%1 ft").arg(realDistance, 0, 'f', 1);
            auto textItem = addText(distanceLabel);
            textItem->setDefaultTextColor(Qt::red);
            textItem->setPos((toolPoints[0] + toolPoints[1]) / 2);
            textItem->setZValue(11);
            measurementItems.append(textItem);

            // Очистка временной линии
            if (tempMeasureLine) {
                removeItem(tempMeasureLine);
                delete tempMeasureLine;
                tempMeasureLine = nullptr;
            }
            if (tempMeasureLabel) {
                removeItem(tempMeasureLabel);
                delete tempMeasureLabel;
                tempMeasureLabel = nullptr;
            }

            toolPoints.clear();
        }
        return;
    }
    QGraphicsScene::mousePressEvent(event);
}

void MapScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (m_toolMode == ToolMode::Measure && toolPoints.size() == 1){
        QPointF currentPos = event->scenePos();
        QLineF line(toolPoints[0], currentPos);

        if (!tempMeasureLine) {
            tempMeasureLine = addLine(line, QPen(Qt::DashLine));
            tempMeasureLine->setZValue(9);
        } else {
            tempMeasureLine->setLine(line);
        }

        double realDistance = line.length() * scaleFactor;
        QString label = QString("%1 ft").arg(realDistance, 0, 'f', 1);

        if (!tempMeasureLabel) {
            tempMeasureLabel = addText(label);
            tempMeasureLabel->setDefaultTextColor(Qt::gray);
            tempMeasureLabel->setZValue(10);
        } else {
            tempMeasureLabel->setPlainText(label);
        }

        tempMeasureLabel->setPos((line.p1() + line.p2()) / 2);
    }
    QGraphicsScene::mouseMoveEvent(event);
}
