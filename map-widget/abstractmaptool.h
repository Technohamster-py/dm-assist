//
// Created by arseniy on 02.05.2025.
//

#ifndef DM_ASSIST_ABSTRACTMAPTOOL_H
#define DM_ASSIST_ABSTRACTMAPTOOL_H

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QObject>

class AbstractMapTool : public QObject{
    Q_OBJECT
public:
    explicit AbstractMapTool(QObject* parent = nullptr) : QObject(parent){}
    virtual ~AbstractMapTool() = default;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) = 0;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) = 0;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) = 0;
    virtual void wheelEvent(QGraphicsSceneWheelEvent *event, QGraphicsScene *scene) = 0;
    virtual void deactivate(QGraphicsScene *scene) = 0;

signals:
    void finished();
};


#endif //DM_ASSIST_ABSTRACTMAPTOOL_H
