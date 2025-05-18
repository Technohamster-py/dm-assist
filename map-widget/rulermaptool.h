//
// Created by arseniy on 02.05.2025.
//

#ifndef DM_ASSIST_RULERMAPTOOL_H
#define DM_ASSIST_RULERMAPTOOL_H

#include "abstractmaptool.h"
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>

/**
 * @class RulerMapTool
 * @brief A tool for measuring distances on a map within a graphical scene.
 *
 * RulerMapTool allows users to measure distances interactively by clicking
 * on the map. Measurements are displayed dynamically as the user moves the
 * mouse and creates permanent distance markers upon clicking.
 *
 * The tool operates as follows:
 * - Left-click to define the endpoints of a measurement.
 * - A dynamic preview displays a temporary line and distance label between
 *   the first clicked point and the current mouse position.
 * - A second click finalizes the measurement, creating a permanent line
 *   and distance label.
 * - Right-click to remove any permanent measurement items.
 * - Upon deactivation, all temporary artifacts are cleared.
 *
 * The tool interacts with a custom graphical scene, utilizing the scale
 * factor provided by the scene to compute accurate distances.
 */
class RulerMapTool : public AbstractMapTool{
public:
    explicit RulerMapTool(QObject *parent = nullptr);

    void mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override {};
    void wheelEvent(QGraphicsSceneWheelEvent *event, QGraphicsScene *scene) override {};
    void deactivate(QGraphicsScene *scene) override;

private:
    QList<QPointF> toolPoints;
    QGraphicsLineItem *tempLine = nullptr;
    QGraphicsTextItem *tempLabel = nullptr;
    QList<QGraphicsItem*> permanentItems;
};


#endif //DM_ASSIST_RULERMAPTOOL_H
