/**
 * @file mapview.h
 * @brief MapView is a QGraphicsView-based widget for RPG map rendering and interaction.
 */

#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QGraphicsView>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QGraphicsPixmapItem>
#include "mapscene.h"
#include "abstractmaptool.h"

/**
 * @class MapView
 * @brief Extends QGraphicsView to provide a customized map display and interaction system.
 *
 * The MapView class serves as a graphical interface for loading, displaying,
 * and interacting with maps using various tools and functionalities such as
 * panning, zooming, and integrating with a custom MapScene.
 * It allows setting active tools to manipulate the map and supports middle mouse
 * drag, custom zoom handling, and customizable scene integration.
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

    [[nodiscard]] MapScene* getScene() const {return scene;};
    bool loadSceneFromFile(const QString& path);


public slots:
    void setActiveTool(AbstractMapTool* tool);

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

    void keyPressEvent(QKeyEvent *event) override;

    void drawForeground(QPainter* painter, const QRectF& rect) override;
private:
    MapScene *scene;                       /**< Scene containing map and tools */
    QGraphicsPixmapItem *mapPixmapItem;   /**< Item displaying the map image */
    bool dragging;                        /**< Whether the view is currently panning */
    QPoint lastMousePos;                  /**< Last mouse position for drag calculation */
    QLabel* heightLabel;
};

#endif // MAPVIEW_H
