//
// Created by arseniy on 02.05.2025.
//

#ifndef DM_ASSIST_RULERMAPTOOL_H
#define DM_ASSIST_RULERMAPTOOL_H

#include "abstractmaptool.h"
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>

class RulerMapTool : public AbstractMapTool{
public:
    explicit RulerMapTool(QObject *parent = nullptr);

    void mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void wheelEvent(QGraphicsSceneWheelEvent *event, QGraphicsScene *scene) override {};
    void deactivate(QGraphicsScene *scene) override;

private:
    QList<QPointF> toolPoints;
    QGraphicsLineItem *tempLine = nullptr;
    QGraphicsTextItem *tempLabel = nullptr;
    QList<QGraphicsItem*> permanentItems;
};


#endif //DM_ASSIST_RULERMAPTOOL_H
