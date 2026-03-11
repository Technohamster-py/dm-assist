#ifndef DM_ASSIST_FOGTOOL_H
#define DM_ASSIST_FOGTOOL_H

#include "abstractmaptool.h"
#include <QGraphicsEllipseItem>

/**
 * @class FogTool
 * @brief A tool for managing fog of war on a map, enabling functionality to hide or reveal specific areas.
 *
 * The FogTool class provides methods to interact with a map scene to manipulate the fog of war.
 * It can hide or reveal specific regions, set the brush size, and update the brush preview
 * in real-time as the mouse interacts with the map.
 */
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

    static void hideAll(QGraphicsScene *scene);
    static void revealAll(QGraphicsScene *scene);

    void mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override {};
    void wheelEvent(QGraphicsSceneWheelEvent *event, QGraphicsScene *scene) override;
    void deactivate(QGraphicsScene *scene) override;

private:
    Mode currentMode = Hide;
    int brushRadius = 30;

    QGraphicsEllipseItem *brushPreview = nullptr;

    void updateBrushPreview(const QPointF &scenePos, QGraphicsScene *scene);
    void removeBrushPreview(QGraphicsScene *scene);
};


#endif //DM_ASSIST_FOGTOOL_H
