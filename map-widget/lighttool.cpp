#include "lighttool.h"
#include "mapscene.h"
#include <QPainter>
#include <QRadialGradient>
#include <QStyleOptionProgressBar>

#include <QDebug>

/**
 * @brief Constructs a LightSourceItem object with the specified parameters.
 *
 * Initializes the properties of the light source, including its radii, color,
 * center position, and associated LightTool. Sets the initial position and
 * z-value of the light source. Configures interaction flags to enable mouse
 * interaction, focusability, and selectability.
 *
 * @param r1 The radius of the bright area of the light source.
 * @param r2 The radius of the dimmed area of the light source.
 * @param color The QColor representing the color of the light source.
 * @param pos The QPointF determining the center position of the light source.
 * @param tool A pointer to the LightTool associated with this light source.
 */
LightSourceItem::LightSourceItem(qreal r1, qreal r2, QColor color, QPointF pos, LightTool *tool)
        : radiusBright(r1), radiusDim(r2), lightColor(color), center(pos), m_tool(tool){
    setPos(center);
    setZValue(mapLayers::Light);

    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
    setFlag(QGraphicsItem::ItemIsFocusable);
}

/**
 * @brief Calculates the bounding rectangle of the light source item.
 *
 * This function returns a QRectF that defines the bounding rectangle of the light source
 * item. The rectangle is centered at the origin (-radiusDim, -radiusDim) and spans
 * a width and height of 2 * radiusDim. It serves as the rectangular area in which
 * the light source is rendered.
 *
 * @return QRectF that represents the bounding rectangle of the item.
 */
QRectF LightSourceItem::boundingRect() const {
    return QRectF(-radiusDim, -radiusDim, 2 * radiusDim, 2 * radiusDim);
}

void LightSourceItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    QRadialGradient gradient(QPointF(0, 0), radiusDim);
    lightColor.setAlpha(220);
    QColor transparent = lightColor;
    transparent.setAlpha(10);

    gradient.setColorAt(0, lightColor);
    gradient.setColorAt(radiusBright / radiusDim, lightColor);
    gradient.setColorAt(1, transparent);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(gradient);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(boundingRect());
}

/**
 * @brief Handles mouse press events for the LightSourceItem.
 *
 * This method intercepts mouse press events and performs different actions
 * depending on the mouse button and modifier keys used:
 *
 * - When the right mouse button is pressed, the LightSourceItem instance is
 *   removed from the scene. If the associated scene is a MapScene and
 *   the associated tool supports auto-updating fog, the fog circle at the
 *   last position of the item is updated before deleting the item.
 *
 * - When the left mouse button is pressed while holding the Control key,
 *   the item enters a dragging state. The initial drag start position is
 *   stored and the event is accepted.
 *
 * - For other mouse press events, the default behavior of the QGraphicsItem
 *   mousePressEvent is executed.
 *
 * @param event The mouse press event containing details about the interaction.
 */
void LightSourceItem::handleMousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        scene()->removeItem(this);
        auto mapScene = dynamic_cast<MapScene*>(scene());
        if (mapScene && m_tool && m_tool->autoUpdateFog()) {
            mapScene->drawFogCircle(lastScenePos, radiusDim + 2, true);
        }
        delete this;
        return;
    }

    if (event->button() == Qt::LeftButton && (event->modifiers() & Qt::ControlModifier)) {
        dragging = true;
        lastScenePos = scenePos();
        dragStart = event->scenePos();
        event->accept();
        return;
    } else {
        QGraphicsItem::mousePressEvent(event);
    }
}

/**
 * @brief Handles the mouse move event for the light source item.
 *
 * This function is invoked when a mouse move event occurs and
 * manages the dragging functionality of the item. If the item
 * is currently being dragged (`dragging` is true), it updates
 * the item's position in the scene based on the difference between
 * the current mouse position and the initial drag start position.
 * The function then updates the drag starting position to reflect
 * the current mouse position and accepts the event to prevent further
 * propagation.
 *
 * @param event Pointer to the QGraphicsSceneMouseEvent containing
 *              details about the mouse move event.
 */
void LightSourceItem::handleMouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (dragging) {
        QPointF delta = event->scenePos() - dragStart;
        setPos(pos() + delta);
        dragStart = event->scenePos();
        event->accept();
        return;
    }
}

/**
 * @brief Handles mouse release events for the light source item.
 *
 * The function checks if the item was in a dragging state:
 * - If `dragging` is true, it ends the dragging operation and interacts with the associated
 *   MapScene to update fog based on the light tool's capabilities.
 *     - If the current tool's `autoUpdateFog` is enabled, it performs an optimized fog update,
 *       drawing circles to represent the updated fog areas.
 *     - If `autoUpdateFog` is not enabled, it simply updates the fog using the item's
 *       current scene position and radius values.
 * - If `dragging` is false, it defers to the default behavior of `QGraphicsItem::mouseReleaseEvent`.
 *
 * @param event The mouse release event triggering this function.
 */
void LightSourceItem::handleMouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (dragging) {
        dragging = false;
        auto mapScene = dynamic_cast<MapScene*>(scene());
        if (mapScene && m_tool && m_tool->autoUpdateFog()) {
            mapScene->drawFogCircle(lastScenePos, radiusDim + 2, true);
            mapScene->drawFogCircle(scenePos(), radiusDim, false);
        } else if (mapScene) {
            mapScene->drawFogCircle(scenePos(), radiusDim, false);
        }
    } else {
        QGraphicsItem::mouseReleaseEvent(event);
    }
}



LightTool::LightTool(QObject *parent) : AbstractMapTool(parent) {}

void LightTool::setBrightRadius(int r1) { m_brightRadius = r1; }
void LightTool::setDimRadius(int r2) { m_dimRadius = r2; }
void LightTool::setColor(QColor c) { m_color = c; }

/**
 * @brief Handles the mouse press event for the LightTool.
 *
 * This method first determines if the mouse click intersects with an existing
 * LightSourceItem within the given scene. If a LightSourceItem is found, its
 * custom mouse press event handler is invoked, and the LightTool does not create
 * any new items. If no LightSourceItem is found, the function checks if the scene
 * is a MapScene.
 *
 * If the scene is a MapScene, a new LightSourceItem is created with the given
 * attributes: bright radius, dim radius, color, and the position of the mouse event.
 * The new item is scaled based on the scene's scaleFactor and added to the scene
 * as an undoable action. Additionally, a visual circle (dim radius) is drawn at
 * the mouse press position to represent the newly created light source.
 *
 * @param event The mouse press event that triggered this function.
 * @param scene A pointer to the graphics scene in which the event occurred.
 */
void LightTool::mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    LightSourceItem* lightSourceItem = dynamic_cast<LightSourceItem*>(scene->itemAt(event->scenePos(), QTransform()));
    if (lightSourceItem) {
        lightSourceItem->handleMousePressEvent(event);
        return;
    }

    auto mapScene = dynamic_cast<MapScene*>(scene);
    if (!mapScene) return;
    double scaleFactor = mapScene->getScaleFactor();
    auto item = new LightSourceItem(m_brightRadius / scaleFactor, m_dimRadius / scaleFactor, m_color, event->scenePos(), this);
    mapScene->addUndoableItem(item);

    mapScene->drawScaledCircle(event->scenePos(), m_dimRadius, false);
}

/**
 * @brief Sets whether the fog should be automatically updated.
 *
 * This function enables or disables the automatic updating of the fog settings.
 * By calling this function and passing a boolean value, the feature to
 * automatically update fog can be toggled.
 *
 * @param enabled A boolean value indicating whether automatic fog update
 *                is enabled. Passing `true` will enable it, while `false`
 *                will disable it.
 */
void LightTool::setAutoUpdateFog(bool enabled) {
    m_autoUpdateFog = enabled;
}

/**
 * @brief Handles mouse move events on a QGraphicsScene.
 *
 * This function processes mouse move events in the scene, allowing interaction with
 * LightSourceItem objects. If the mouse move event occurs on a LightSourceItem, the
 * event is delegated to the LightSourceItem for further handling.
 *
 * @param event The mouse move event containing information about the cursor's movement.
 * @param scene The QGraphicsScene in which the event is occurring.
 *
 * If the mouse cursor is over a LightSourceItem in the scene, that item's
 * handleMouseMoveEvent method will be invoked, passing along the event for
 * custom processing specific to the LightSourceItem.
 */
void LightTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    LightSourceItem* lightSourceItem = dynamic_cast<LightSourceItem*>(scene->itemAt(event->scenePos(), QTransform()));
    if (lightSourceItem) {
        lightSourceItem->handleMouseMoveEvent(event);
        return;
    }
}

/**
 * Handles the mouse release event in the context of the LightTool.
 *
 * This function is triggered when a mouse button is released within the scene where the tool operates.
 * It checks if the mouse release action interacts with a LightSourceItem within the scene. If such an
 * item is found at the location of the mouse release event, the event is forwarded to the corresponding
 * LightSourceItem for handling.
 *
 * @param event Pointer to the QGraphicsSceneMouseEvent representing the mouse release event.
 * @param scene Pointer to the QGraphicsScene where the event occurred.
 */
void LightTool::mouseReleaseEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    LightSourceItem* lightSourceItem = dynamic_cast<LightSourceItem*>(scene->itemAt(event->scenePos(), QTransform()));
    if (lightSourceItem) {
        lightSourceItem->handleMouseReleaseEvent(event);
        return;
    }
}
