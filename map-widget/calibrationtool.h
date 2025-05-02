//
// Created by arseniy on 02.05.2025.
//

#ifndef DM_ASSIST_CALIBRATIONTOOL_H
#define DM_ASSIST_CALIBRATIONTOOL_H

#include "abstractmaptool.h"

class CalibrationTool : public AbstractMapTool{
public:
    explicit CalibrationTool(QObject *parent = nullptr);

    void mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void deactivate(QGraphicsScene *scene) override;

private:
    QList<QPointF> points;
    QGraphicsLineItem *previewLine = nullptr;
};


#endif //DM_ASSIST_CALIBRATIONTOOL_H
