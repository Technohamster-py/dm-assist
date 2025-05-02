/**
 * @file mapscene.h
 * @brief MapScene manages map elements and tool interactions in the RPG map viewer.
 */

#ifndef MAPSCENE_H
#define MAPSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>

enum ToolMode{
    None,
    Calibration,
    Measure
};

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
    void setToolMode(ToolMode mode);

private:
    ToolMode m_toolMode = ToolMode::None;
    QList<QPointF> toolPoints;
    QGraphicsLineItem *tempMeasureLine = nullptr;
    QGraphicsTextItem *tempMeasureLabel = nullptr;
    QList<QGraphicsItem*> measurementItems; // Храним все линии и подписи
    double scaleFactor = 1.0;           ///< Масштаб

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif // MAPSCENE_H
