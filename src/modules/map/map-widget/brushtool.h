#ifndef DM_ASSIST_BRUSHTOOL_H
#define DM_ASSIST_BRUSHTOOL_H

#include "abstractmaptool.h"
#include <QPainterPath>

class BrushTool : public AbstractMapTool{
Q_OBJECT
public:
    explicit BrushTool(QObject* parent = nullptr);

    void setColor(const QColor& color);
    void setOpacity(qreal alpha);
    void setBrushSize(int px);

    void mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void wheelEvent(QGraphicsSceneWheelEvent *event, QGraphicsScene *scene) override;
    void deactivate(QGraphicsScene *scene) override;

    static void clearAll(QGraphicsScene *scene);

private:
    QPainterPath path;
    QGraphicsPathItem* previewItem = nullptr;
    QColor color = Qt::red;
    qreal opacity = 0.5;
    int brushSize = 20;

    void updateBrushPreview(const QPointF &scenePos, QGraphicsScene *scene);
};



#endif //DM_ASSIST_BRUSHTOOL_H
