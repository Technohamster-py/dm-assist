#ifndef DM_ASSIST_LIGHTTOOL_H
#define DM_ASSIST_LIGHTTOOL_H

#include <QGraphicsItem>
#include <QColor>

#include "abstractmaptool.h"

class LightSourceItem : public QGraphicsItem {
public:
    LightSourceItem(qreal r1, qreal r2, QColor color, QPointF position);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    qreal radiusBright;
    qreal radiusDim;
    QColor lightColor;
    QPointF center;

    bool dragging = false;
    QPointF dragStart;
};


class LightTool : public AbstractMapTool {
Q_OBJECT
public:
    explicit LightTool(QObject *parent = nullptr);

    void setBrightRadius(int r1);
    void setDimRadius(int r2);
    void setColor(QColor color);

    QColor color() const {return m_color;};
    int brightRadius() const {return m_brightRadius;};
    int dinRadius() const {return m_dimRadius;};

    void mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {};
    void wheelEvent(QGraphicsSceneWheelEvent *event, QGraphicsScene *scene) override {};
    void deactivate(QGraphicsScene *scene) override {};
private:
    int m_brightRadius = 50;
    int m_dimRadius = 100;
    QColor m_color = QColor(255, 255, 180); // тёплый свет по умолчанию
};



#endif //DM_ASSIST_LIGHTTOOL_H
