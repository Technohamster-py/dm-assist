/**
 * @file mapscene.h
 * @brief MapScene manages map elements and tool interactions in the RPG map viewer.
 */

#ifndef MAPSCENE_H
#define MAPSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>

#include "abstractmaptool.h"

/**
 * @class MapScene
 * @brief A scene for displaying and interacting with RPG map elements.
 */
class MapScene : public QGraphicsScene
{
Q_OBJECT

public:
    /**
     * @brief Constructs a new MapScene.
     * @param parent Optional parent QObject
     */
    explicit MapScene(QObject *parent = nullptr);
    void setActiveTool(AbstractMapTool* tool);

    void setScaleFactor(double factor);
    [[nodiscard]] double getScaleFactor() const {return m_scaleFactor;};

    void initializeFog(const QSize &size);
    void drawFogCircle(const QPointF &scenePos, int radius, bool hide);
    QImage getFogImage() const {return fogImage;};
    QPixmap getMapPixmap() const;
    void setFogOpacity(qreal opacity); // 0.0–1.0

    void wheelEvent(QGraphicsSceneWheelEvent *event) override;
signals:
    void fogUpdated(const QImage &fogImage);
private:
    AbstractMapTool* m_activeTool = nullptr;
    double m_scaleFactor = 1.0;           ///< Масштаб

    QGraphicsPixmapItem *fogItem = nullptr;
    QImage fogImage;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif // MAPSCENE_H
