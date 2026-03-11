#ifndef DM_ASSIST_CALIBRATIONTOOL_H
#define DM_ASSIST_CALIBRATIONTOOL_H

#include "abstractmaptool.h"

/**
 * @class CalibrationTool
 * @brief A tool for calibrating the scale of a map by selecting two points and defining their real-world distance.
 *
 * CalibrationTool is a specialized implementation of AbstractMapTool that provides
 * functionality to measure distances on a graphical scene and configure the map's scale factor.
 * Users select two points on the scene, input the real-world distance between them, and the tool
 * computes and sets the appropriate scale factor for the map.
 */
class CalibrationTool : public AbstractMapTool{
Q_OBJECT
public:
    explicit CalibrationTool(QObject *parent = nullptr);

    void mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override {};
    void wheelEvent(QGraphicsSceneWheelEvent *event, QGraphicsScene *scene) override {};
    void deactivate(QGraphicsScene *scene) override;

private:
    QList<QPointF> points;
    QGraphicsLineItem *previewLine = nullptr;
};


#endif //DM_ASSIST_CALIBRATIONTOOL_H
