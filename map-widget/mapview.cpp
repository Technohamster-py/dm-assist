/**
 * @file mapview.cpp
 * @brief Implementation of MapView for viewing and interacting with RPG maps.
 */

#include "mapview.h"
#include <QMouseEvent>
#include <QGraphicsPixmapItem>
#include <QScrollBar>

/**
 * @brief Constructs a new MapView and initializes the scene.
 */
MapView::MapView(QWidget *parent)
        : QGraphicsView(parent), dragging(false)
{
    scene = new MapScene(this);
    setScene(scene);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(AnchorUnderMouse);
    mapPixmapItem = nullptr;
}

/**
 * @brief Loads the map image and adds it to the scene.
 * @param filePath Path to the map image file.
 */
void MapView::loadMapImage(const QString &filePath)
{
    QPixmap pixmap(filePath);
    if (!pixmap.isNull()) {
        if (mapPixmapItem) {
            scene->removeItem(mapPixmapItem);
            delete mapPixmapItem;
        }
        mapPixmapItem = scene->addPixmap(pixmap);
        mapPixmapItem->setZValue(-100); // ниже всех остальных элементов
        scene->setSceneRect(pixmap.rect());
    }
}

void MapView::wheelEvent(QWheelEvent *event)
{
    const double scaleFactor = 1.15;
    if (event->angleDelta().y() > 0)
        scale(scaleFactor, scaleFactor);
    else
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
}

void MapView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        dragging = true;
        lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    } else {
        QGraphicsView::mousePressEvent(event);
    }
}

void MapView::mouseMoveEvent(QMouseEvent *event)
{
    if (dragging) {
        QPoint delta = event->pos() - lastMousePos;
        lastMousePos = event->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        event->accept();
    } else {
        QGraphicsView::mouseMoveEvent(event);
    }
}

void MapView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton && dragging) {
        dragging = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
    } else {
        QGraphicsView::mouseReleaseEvent(event);
    }
}
