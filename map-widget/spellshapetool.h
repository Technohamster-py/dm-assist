#ifndef DM_ASSIST_SPELLSHAPETOOL_H
#define DM_ASSIST_SPELLSHAPETOOL_H

#include "abstractmaptool.h"
#include "spellshapeitem.h"

class SpellShapeTool : public AbstractMapTool {
Q_OBJECT
public:
    explicit SpellShapeTool(QObject *parent = nullptr);

    void mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) = 0;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) = 0;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override {};
    void wheelEvent(QGraphicsSceneWheelEvent *event, QGraphicsScene *scene) override {};
    void deactivate(QGraphicsScene *scene) { clearPreview(scene);};

    void setColor(QColor c);

protected:
    QColor color = Qt::cyan;
    QGraphicsItem *previewShape = nullptr;
    QGraphicsTextItem *previewLabel = nullptr;

    QPointF firstPoint;
    bool hasFirstPoint = false;

    void clearPreview(QGraphicsScene *scene);
    bool clearShapeAt(QGraphicsScene *scene, QPointF point);
};


class LineShapeTool : public SpellShapeTool {
Q_OBJECT
public:
    void mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene);
};

class CircleShapeTool : public SpellShapeTool {
public:
    void mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
private:
    QRectF circleRect(QPointF point);
};


class TriangleShapeTool : public SpellShapeTool {
public:
    void mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;

private:
    QPointF mirrorPoint(const QPointF &p);
};

class SquareShapeTool : public SpellShapeTool {
public:
    void mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;

private:
    QPolygonF buildSquare(const QPointF &p1, const QPointF &p2);
};


#endif //DM_ASSIST_SPELLSHAPETOOL_H
