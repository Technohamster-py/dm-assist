#ifndef DM_ASSIST_ABSTRACTMAPTOOL_H
#define DM_ASSIST_ABSTRACTMAPTOOL_H

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QObject>

/**
 * @brief The AbstractMapTool class provides an interface for implementing
 *        custom tools to interact with a map in a graphical scene.
 *
 * This class is an abstract base class designed to be extended by other
 * classes to handle various input events (mouse events, wheel events) and
 * provide related functionalities within a QGraphicsScene.
 *
 * The AbstractMapTool class also includes a mechanism for signaling when
 * the tool's operation is complete via the `finished` signal.
 */
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
