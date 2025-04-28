/**
 * @file mapview.h
 * @brief MapView is a QGraphicsView-based widget for RPG map rendering and interaction.
 */

#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QGraphicsView>
#include <QWheelEvent>
#include <QGraphicsPixmapItem>
#include "mapscene.h"

/**
 * @class MapView
 * @brief A view for displaying and interacting with a map image.
 */
class MapView : public QGraphicsView
{
Q_OBJECT

public:
    /**
     * @brief Constructor for MapView.
     * @param parent Optional parent widget
     */
    explicit MapView(QWidget *parent = nullptr);

    /**
     * @brief Loads a map image from file and displays it in the scene.
     * @param filePath Path to the image file
     */
    void loadMapImage(const QString &filePath);

protected:
    /**
     * @brief Handles mouse wheel events for zooming.
     * @param event Pointer to wheel event
     */
    void wheelEvent(QWheelEvent *event) override;

    /**
     * @brief Handles mouse press events for enabling drag/pan.
     * @param event Pointer to mouse event
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief Handles mouse move events for dragging.
     * @param event Pointer to mouse event
     */
    void mouseMoveEvent(QMouseEvent *event) override;

    /**
     * @brief Handles mouse release events to stop panning.
     * @param event Pointer to mouse event
     */
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    MapScene *scene;                       /**< Scene containing map and tools */
    QGraphicsPixmapItem *mapPixmapItem;   /**< Item displaying the map image */
    bool dragging;                        /**< Whether the view is currently panning */
    QPoint lastMousePos;                  /**< Last mouse position for drag calculation */
};

#endif // MAPVIEW_H
