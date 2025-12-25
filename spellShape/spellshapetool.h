#ifndef DM_ASSIST_SPELLSHAPETOOL_H
#define DM_ASSIST_SPELLSHAPETOOL_H

#include "map-widget/areashapetool.h"

/**
 * @class SpellShapeTool
 * @brief A tool designed for handling shape-based operations within a graphical map-like environment.
 *
 * This class provides mechanisms for creating, manipulating, and removing graphical shapes in a
 * scene. It derives from AbstractMapTool and implements various mouse-driven interactions and
 * preview functionalities for shape-based content. Each instance of this tool modifies shapes
 * by interacting with a QGraphicsScene and utilizes color and preview features.
 *
 * The tool is abstract and requires derived classes to implement specific behavior for certain mouse events.
 */
class SpellShapeTool : public AreaShapeTool {
Q_OBJECT
public:
    explicit SpellShapeTool(QObject *parent = nullptr);
    void rightClickEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene);
protected:
    QPointF firstPoint;
    bool hasFirstPoint = false;
};


/**
 * @class LineShapeTool
 * @brief A tool for drawing line shapes on a map with interactive feedback.
 *
 * Inherits from SpellShapeTool and provides functionality for creating and previewing
 * straight line shapes with user interaction in a QGraphicsScene. It supports real-time
 * distance calculation and allows clearing or placing lines using mouse events.
 */
class LineShapeTool : public SpellShapeTool {
Q_OBJECT
public:
    void mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
};

/**
 * @class CircleShapeTool
 * @brief A tool for drawing circular shapes within a QGraphicsScene.
 *
 * This class extends the SpellShapeTool and provides specific functionality
 * for drawing circles in a graphical scene. Users can press and drag the mouse
 * to define circular shapes, with support for previews and measurements.
 */
class CircleShapeTool : public SpellShapeTool {
public:
    void mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
private:
    QRectF circleRect(QPointF point);
};


/**
 * @class TriangleShapeTool
 * @brief A tool for creating and previewing triangle shapes within a QGraphicsScene.
 *
 * This class extends SpellShapeTool to provide functionality for drawing
 * triangular shapes on a graphics scene. It supports both the creation of
 * permanent triangle shapes and dynamic previews while interacting with the scene.
 */
class TriangleShapeTool : public SpellShapeTool {
public:
    void mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;

private:
    QPolygonF buildTriangle(const QPointF &hPoint);
};

/**
 * @class SquareShapeTool
 * @brief A tool for creating and previewing square shapes in a graphical scene.
 *
 * This class derives from SpellShapeTool and provides specific implementations for handling
 * mouse interactions to create square shapes. The square is defined based on two points
 * representing a diagonal, and the tool supports live previews of the square during mouse
 * dragging.
 */
class SquareShapeTool : public SpellShapeTool {
public:
    void mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;

private:
    static QPolygonF buildSquare(const QPointF &p1, const QPointF &p2);
};


#endif //DM_ASSIST_SPELLSHAPETOOL_H
