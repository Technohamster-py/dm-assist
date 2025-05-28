#ifndef DM_ASSIST_LIGHTTOOL_H
#define DM_ASSIST_LIGHTTOOL_H

#include <QGraphicsItem>
#include <QColor>

#include "abstractmaptool.h"
class LightTool;

/**
 * @class LightSourceItem
 * @brief A graphical representation of a light source on a QGraphicsScene.
 *
 * The LightSourceItem class represents a light source with adjustable radii and color.
 * It supports mouse interaction for movement and removal, and integrates with
 * a LightTool to manage fog effects.
 */
class LightSourceItem : public QGraphicsItem {
public:
    LightSourceItem(qreal r1, qreal r2, QColor color, QPointF position, LightTool *tool);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

    void handleMousePressEvent(QGraphicsSceneMouseEvent *event);
    void handleMouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void handleMouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    qreal brightRadius() const {return radiusBright;};
    qreal dimRadius() const {return radiusDim;};
    QColor color() const {return lightColor;};

private:
    qreal radiusBright;
    qreal radiusDim;
    QColor lightColor;
    QPointF center;

    LightTool* m_tool;
    QPointF lastScenePos;

    bool dragging = false;
    QPointF dragStart;
};


/**
 * @class LightTool
 * @brief A tool for manipulating light sources on a map scene.
 *
 * LightTool provides functionality to add and manipulate light sources
 * with configurable properties such as brightness radius, dim radius, and color.
 * It also handles interactions with light source objects using mouse events.
 */
class LightTool : public AbstractMapTool {
Q_OBJECT
public:
    explicit LightTool(QObject *parent = nullptr);

    void setBrightRadius(int r1);
    void setDimRadius(int r2);
    void setColor(QColor color);

    QColor color() const {return m_color;};
    int brightRadius() const {return m_brightRadius;};
    int dimRadius() const {return m_dimRadius;};

    bool autoUpdateFog() const {return m_autoUpdateFog;};
    void setAutoUpdateFog(bool enabled);

    void mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void wheelEvent(QGraphicsSceneWheelEvent *event, QGraphicsScene *scene) override {};
    void deactivate(QGraphicsScene *scene) override {};
private:
    int m_brightRadius = 20;
    int m_dimRadius = 40;
    QColor m_color = QColor(255, 255, 180);

    bool m_autoUpdateFog = false;
};



#endif //DM_ASSIST_LIGHTTOOL_H
