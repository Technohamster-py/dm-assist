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
 * @brief Constructs a MapView object, initializing the map display and interaction features.
 *
 * The constructor sets up the graphical environment by creating a new MapScene instance and
 * associating it with the MapView. It applies rendering hints for improved graphical quality,
 * sets the drag mode for panning functionality, and establishes the transformation anchor
 * to enable smooth zooming and interaction centered under the mouse pointer.
 * The mapPixmapItem pointer is initialized to nullptr, and dragging is disabled by default.
 *
 * @param parent An optional parent widget for MapView.
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
 * @brief Loads and displays a map image in the custom scene.
 *
 * This method takes a file path to an image, loads it into a QPixmap,
 * and displays it within the associated MapScene. If an image is already displayed,
 * it removes the current pixmap from the scene before replacing it with the new one.
 * The new image is then set to the lowest Z-value to ensure it appears below
 * any other graphical elements.
 *
 * The dimensions of the map image are used to initialize fog in the scene,
 * and the scene rectangle is adjusted to match the bounding rectangle
 * of the loaded image.
 *
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
        mapPixmapItem->setZValue(mapLayers::Background); // ниже всех остальных элементов
        scene->initializeFog(pixmap.size());
        scene->setSceneRect(mapPixmapItem->boundingRect());
    }
}

/**
 * @brief Handles the mouse wheel event for zooming functionality in the map view.
 *
 * This function overrides the QGraphicsView::wheelEvent to implement both
 * custom zooming behavior and the forwarding of the wheel event to the associated scene
 * for additional handling. If the Control key is pressed during the wheel event,
 * the event is forwarded to the scene with the associated scene position and delta.
 *
 * If the Control key is not pressed, the function performs a zoom action on the view
 * based on the delta of the scroll wheel. The zoom operation increases or decreases
 * the scale factor accordingly.
 *
 * @param event Pointer to the QWheelEvent containing information about the wheel event.
 */
void MapView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        QPointF scenePos = mapToScene(event->position().toPoint());

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

/**
 * @brief Handles the mouse press event to enable custom interactions in the map view.
 *
 * This function overrides the default mouse press behavior. If the middle mouse
 * button is pressed, it activates "dragging" mode, storing the current mouse position
 * for subsequent movement calculations, and changes the cursor to a closed hand symbol.
 * The event is accepted to prevent further propagation. For other mouse button presses,
 * the event is forwarded to the base class implementation for default processing.
 *
 * @param event Pointer to the QMouseEvent that contains details about the mouse press event.
 */
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

/**
 * @brief Handles mouse movement during a drag operation or delegates to the base implementation.
 *
 * This method is called when a mouse move event occurs within the MapView.
 * - If the user is currently dragging (`dragging` is true), it calculates the delta
 *   movement of the mouse from the last recorded position (`lastMousePos`) and accordingly
 *   scrolls the view using the horizontal and vertical scroll bars.
 * - Updates `lastMousePos` to the current mouse position.
 * - Marks the event as handled by calling `event->accept()`.
 * - If not dragging, passes the event to the base class implementation (`QGraphicsView::mouseMoveEvent`).
 *
 * @param event Pointer to the QMouseEvent object containing event details
 * such as cursor position.
 */
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

/**
 * @brief Handles the mouse release event to end panning or forward it to the default handler.
 *
 * This function checks if the middle mouse button was released while the view was in a dragging state.
 * If so, it stops the dragging operation, resets the cursor to the default arrow cursor, and accepts the event.
 * For all other cases, it calls the base class implementation to handle the event.
 *
 * @param event Pointer to the QMouseEvent containing information about the mouse release action.
 */
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

/**
 * @brief Sets the active map tool and updates the drag mode for the view.
 *
 * This method sets the currently active tool for interacting with the map.
 * If a tool is provided, the drag mode is disabled to allow the tool to
 * handle interactions. Otherwise, the drag mode is set to allow panning
 * using the scroll-hand drag mode.
 *
 * The tool is also set as active in the associated MapScene object.
 *
 * @param tool A pointer to an AbstractMapTool object representing the tool
 *             to be activated. Passing nullptr will deactivate the current tool.
 */
void MapView::setActiveTool(AbstractMapTool* tool) {
    if (tool)
        setDragMode(QGraphicsView::NoDrag);
    else
        setDragMode(QGraphicsView::ScrollHandDrag);
    scene->setActiveTool(tool);
}

/**
 * @brief Handles key press events within the MapView context.
 *
 * Intercepts key presses to provide additional functionality within the
 * custom map interaction system. Specifically, handles the escape key
 * press to deactivate any currently active tool by setting it to `nullptr`.
 * Passes unhandled key events to the base QGraphicsView implementation
 * for default processing.
 *
 * @param event Pointer to the key event object containing event data.
 */
void MapView::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape){
        setActiveTool(nullptr);
    }
    QGraphicsView::keyPressEvent(event);
}

/**
 * @brief Loads a scene from a file into the current map view.
 *
 * Attempts to load a scene from the specified file path using the associated
 * MapScene instance. The method returns true if the scene is successfully
 * loaded. If an error occurs during loading, a message box is displayed to
 * notify the user, and the method returns false.
 *
 * Error conditions handled include:
 * - File cannot be opened (qmapErrorCodes::FileOpenError)
 * - File does not have the correct signature (qmapErrorCodes::FileSignatureError)
 * - JSON parsing issues in the file (qmapErrorCodes::JsonParseError)
 * - Any other unknown errors
 *
 * @param path The file path to load the scene from.
 * @return true if the scene is successfully loaded, false otherwise.
 */
bool MapView::loadSceneFromFile(const QString &path) {
    int errorCode = scene->loadFromFile(path);

    switch (errorCode) {
        case qmapErrorCodes::NoError:
            return true;
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
    return false;
}
