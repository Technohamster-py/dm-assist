//
// Created by arseniy on 03.05.2025.
//

#ifndef DM_ASSIST_FOGTOOL_H
#define DM_ASSIST_FOGTOOL_H

#include "abstractmaptool.h"
#include <QGraphicsEllipseItem>

class FogTool : public AbstractMapTool{
Q_OBJECT
public:
    enum Mode
    {
    Hide,
    Reveal
    };
    explicit FogTool(QObject* parent = nullptr);
    void setMode(Mode mode);
    void setRadius(int radius);

    void mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void deactivate(QGraphicsScene *scene) override;

private:
    Mode currentMode = Hide;
    int brushRadius = 30;

    QGraphicsEllipseItem *brushPreview = nullptr;

    void updateBrushPreview(const QPointF &scenePos, QGraphicsScene *scene);
    void removeBrushPreview(QGraphicsScene *scene);
};


#endif //DM_ASSIST_FOGTOOL_H
