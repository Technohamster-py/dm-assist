/**
 * @file mapview.cpp
 * @brief Implementation of MapView for viewing and interacting with RPG maps.
 */

#include "mapview.h"
#include <QMouseEvent>
#include <QGraphicsPixmapItem>
#include <QScrollBar>
#include <QMessageBox>

/**
 * @brief Constructs a new MapView and initializes the scene.
 */
MapView::MapView(QWidget *parent)
        : QGraphicsView(parent), dragging(false)
{
    scene = new MapScene(this);
    setScene(scene);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setRenderHint(QPainter::Antialiasing);
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
        scene->initializeFog(pixmap.size());
        scene->setSceneRect(mapPixmapItem->boundingRect());
    }
}

void MapView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        QPointF scenePos = mapToScene(event->posF().toPoint());

        QGraphicsSceneWheelEvent sceneEvent(QEvent::GraphicsSceneWheel);
        sceneEvent.setScenePos(scenePos);
        sceneEvent.setDelta(event->angleDelta().y());
        sceneEvent.setModifiers(event->modifiers());
        sceneEvent.setButtons(event->buttons());

        scene->wheelEvent(&sceneEvent);

        event->accept();
        return;
    }

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

void MapView::setActiveTool(AbstractMapTool* tool) {
    if (tool)
        setDragMode(QGraphicsView::NoDrag);
    else
        setDragMode(QGraphicsView::ScrollHandDrag);
    scene->setActiveTool(tool);
}

void MapView::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape){
        setActiveTool(nullptr);
    }
    QGraphicsView::keyPressEvent(event);
}

void MapView::loadSceneFromFile(const QString &path) {
    int errorCode = scene->loadFromFile(path);

    switch (errorCode) {
        case qmapErrorCodes::NoError:
            break;
        case qmapErrorCodes::FileOpenError:
            QMessageBox::critical(this, "Open file error", QString("Can't open file %1").arg(path));
            break;
        case qmapErrorCodes::FileSignatureError:
            QMessageBox::critical(this, "Open file error", "File is not DM-assist map");
            break;
        case qmapErrorCodes::JsonParseError:
            QMessageBox::critical(this, "Open file error", "Json parse error");
            break;
        default:
            QMessageBox::critical(this, "Open file error", "Unknown error");
            break;
    }
}
