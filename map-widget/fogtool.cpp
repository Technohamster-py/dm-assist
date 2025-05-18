#include "fogtool.h"
#include "mapscene.h"

FogTool::FogTool(QObject *parent) : AbstractMapTool(parent) {}

void FogTool::setMode(FogTool::Mode mode) {
    currentMode = mode;
}

void FogTool::setRadius(int radius) {
    brushRadius = radius;
}

/**
 * @brief Handles the mouse press event for the FogTool.
 *
 * This method is invoked when the user presses the mouse button while the FogTool
 * is active in the specified scene. It performs the following actions:
 * - Updates the visual "brush preview" to reflect the current tool state and position.
 * - If the given scene is a MapScene, it applies a fog effect around the position
 *   where the mouse was pressed. The fog is drawn either to hide or reveal the area
 *   depending on the currentMode set for the tool.
 *
 * @param event The mouse press event containing the scene position and other data.
 * @param scene The graphics scene in which the event occurred.
 */
void FogTool::mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    updateBrushPreview(event->scenePos(), scene);

    auto mapScene = dynamic_cast<MapScene*>(scene);
    if (mapScene) {
        mapScene->drawFogCircle(event->scenePos(), brushRadius, currentMode == Hide);
    }
}

/**
 * @brief Handles mouse move events for the fog tool.
 *
 * Updates the brush preview to follow the current mouse position and checks for interaction
 * with the MapScene. If the left mouse button is pressed, the fog tool draws a fog circle
 * on the scene at the mouse position. The behavior of the fog tool depends on the current
 * mode, either hiding or revealing the fog.
 *
 * @param event Pointer to the QGraphicsSceneMouseEvent representing the mouse move event.
 * @param scene Pointer to the QGraphicsScene where the event occurred, expected to be a MapScene.
 */
void FogTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    updateBrushPreview(event->scenePos(), scene);

    auto mapScene = dynamic_cast<MapScene*>(scene);
    if (mapScene && (event->buttons() & Qt::LeftButton)) {
        mapScene->drawFogCircle(event->scenePos(), brushRadius, currentMode == Hide);
    }
}

/**
 * @brief Updates the position and size of the brush preview on the scene.
 *
 * This function is responsible for updating the visual representation of the
 * brush preview within the specified scene. If no brush preview exists, a new
 * QGraphicsEllipseItem will be created and added to the scene. If the brush
 * preview already exists, its position and size will be updated based on the
 * provided scene coordinates and the current brush radius.
 *
 * @param scenePos The position in the scene where the brush preview should be updated.
 * @param scene The QGraphicsScene instance where the brush preview is displayed.
 *              If this parameter is null, the function will return without performing any action.
 */
void FogTool::updateBrushPreview(const QPointF &scenePos, QGraphicsScene *scene) {
    if (!scene) return;

    QPointF topLeft = scenePos - QPointF(brushRadius, brushRadius);
    QSizeF size(brushRadius * 2, brushRadius * 2);

    if (!brushPreview) {
        brushPreview = scene->addEllipse(QRectF(topLeft, size),
                                         QPen(QBrush(Qt::DashLine), 1.5, Qt::DashLine),
                                         Qt::NoBrush);
        brushPreview->setZValue(100); // выше всего
    } else {
        brushPreview->setRect(QRectF(topLeft, size));
    }
}

/**
 * @brief Removes the brush preview item from the specified graphics scene.
 *
 * This function checks if a brush preview exists and a valid scene is provided.
 * If both conditions are satisfied, the brush preview graphical item is removed
 * from the scene, and the associated memory is released.
 *
 * @param scene The QGraphicsScene from which the brush preview will be removed.
 *              If the scene is null or there is no active brush preview, the function
 *              does nothing.
 */
void FogTool::removeBrushPreview(QGraphicsScene *scene) {
    if (brushPreview && scene) {
        scene->removeItem(brushPreview);
        delete brushPreview;
        brushPreview = nullptr;
    }
}

/**
 * @brief Deactivates the FogTool by removing the brush preview from the given scene.
 *
 * This function is called to deactivate the FogTool's operations. It ensures that
 * any visual representation of the brush (indicating the current radius or mode) is
 * removed from the provided QGraphicsScene. The removal is managed by calling the
 * `removeBrushPreview` method.
 *
 * @param scene A pointer to the QGraphicsScene where the tool's brush preview should be removed.
 *              If the scene is null, no action is performed.
 */
void FogTool::deactivate(QGraphicsScene *scene) {
    removeBrushPreview(scene);
}

/**
 * @brief Handles the mouse wheel event to adjust the brush radius in the fog editing tool.
 *
 * This function is called when a wheel event is triggered in the scene. It modifies the
 * `brushRadius` value based on the wheel's rotation, increasing or decreasing it by a value of 5.
 * The `brushRadius` value is clamped between 5 and 300. After updating the radius, it also
 * triggers an update to the brush preview to reflect the new size in the scene at the wheel
 * event's scene position. Finally, the event is accepted to indicate that it has been handled.
 *
 * @param event A pointer to the QGraphicsSceneWheelEvent that contains details of the wheel event.
 * @param scene A pointer to the QGraphicsScene where the event occurred and brush preview is updated.
 */
void FogTool::wheelEvent(QGraphicsSceneWheelEvent *event, QGraphicsScene *scene) {
    int delta = event->delta() > 0 ? 5 : -5;
    brushRadius = std::clamp(brushRadius + delta, 5, 300);
    updateBrushPreview(event->scenePos(), scene);
    event->accept();
}

/**
 * @brief Hides the entire scene with fog.
 *
 * This method applies a fog effect over the entire area of the provided graphics scene.
 * If the supplied scene is not of type MapScene, the function will terminate without action.
 *
 * @param scene A pointer to the QGraphicsScene instance on which the fog should be applied.
 *              If nullptr, the function exits without performing any operation.
 */
void FogTool::hideAll(QGraphicsScene *scene) {
    if (!scene) return;

    auto mapScene = dynamic_cast<MapScene*>(scene);
    if (!mapScene) return;

    QRectF bounds = scene->sceneRect();
    QPainterPath path;
    path.addRect(bounds);
    mapScene->drawFogPath(path, true);
}

/**
 * @brief Reveals all fog in the given scene.
 *
 * This method clears all fog layers in the provided QGraphicsScene,
 * provided the scene is of type MapScene. If the passed scene is
 * nullptr or cannot be cast to a MapScene, the function does nothing.
 *
 * @param scene Pointer to a QGraphicsScene that represents the scene where the fog should be cleared.
 */
void FogTool::revealAll(QGraphicsScene *scene) {
    if (!scene) return;
    auto mapScene = dynamic_cast<MapScene*>(scene);
    if (!mapScene) return;

    mapScene->clearFog();
}
