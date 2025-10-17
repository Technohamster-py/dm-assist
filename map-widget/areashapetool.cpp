#include "areashapetool.h"
#include "mapscene.h"
#include "effectgrapchicsitem.h"


/**
 * @brief Clears the preview elements from the provided QGraphicsScene.
 *
 * This function removes and deletes the `previewShape` and `previewLabel`
 * objects from the specified graphics scene. After deletion, the respective
 * pointers are set to `nullptr` to ensure they are not accessed again.
 *
 * @param scene A pointer to the QGraphicsScene from which the preview elements
 *              should be removed.
 */
void AreaShapeTool::clearPreview(QGraphicsScene *scene) {
    if (previewShape) {
        scene->removeItem(previewShape);
        delete previewShape;
        previewShape = nullptr;
    }
    if (previewLabel) {
        scene->removeItem(previewLabel);
        delete previewLabel;
        previewLabel = nullptr;
    }
}

/**
 * @brief Sets the shape color and updates its transparency.
 *
 * This function updates the `color` property of the `SpellShapeTool` class.
 * It assigns the provided color to the `color` variable and then ensures
 * that the alpha (transparency) value is set to 180. This is used to ensure
 * a consistent semi-transparency for rendering purposes.
 *
 * @param c The new color to be assigned to the shape, of type `QColor`.
 */
void AreaShapeTool::setColor(QColor c) {
    color = c;
    color.setAlpha(180);
}

/**
 * @brief Removes a shape at a specified position in the given scene.
 *
 * This function attempts to clear a shape located at the specified point
 * within the provided QGraphicsScene. It iterates over the items at the
 * given point, validating their z-value to ensure they are within a
 * designated range. If a valid item is found, it is removed as an undoable
 * action using a MapScene-specific method.
 *
 * @param scene Pointer to the QGraphicsScene where the operation is performed.
 * @param point The position in the scene where the shape should be cleared.
 * @return True if a shape was successfully removed; false otherwise.
 */
bool AreaShapeTool::clearShapeAt(QGraphicsScene *scene, QPointF point) {
    QList<QGraphicsItem*> items = scene->items(point);

    for (QGraphicsItem* item : items) {
        if (!item) return false;

        if (item->zValue() != mapLayers::Shapes) continue;


        dynamic_cast<MapScene *>(scene)->removeUndoableItem(item);
        return true;
    }
    return false;
}

QBrush AreaShapeTool::getBrush() {
    QPixmap pixmap(currentTextureName);
    QBrush brush;
    if (!pixmap.isNull()){
        brush = QBrush(pixmap);
        brush.setStyle(Qt::TexturePattern);
        brush.setColor(QColor(255, 255, 255, 50));
    } else {
        brush = QBrush(color, Qt::Dense4Pattern);
    }
    return brush;
}

AreaShapeTool::AreaShapeTool(QObject *parent) : AbstractMapTool(parent) {
    color.setAlpha(180);
}

void LassoTool::mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (event->button() == Qt::RightButton) {
        auto* regionItem = scene->itemAt(event->scenePos(), QTransform());
        if (regionItem->zValue() == mapLayers::Shapes){
            scene->removeItem(regionItem);
            delete regionItem;
        }
        return;
    }
    path = QPainterPath(event->scenePos());
    preview = new QGraphicsPathItem();

    preview->setPen(QPen(Qt::darkGray, 2, Qt::DashLine));
    preview->setZValue(mapLayers::Brush);
    scene->addItem(preview);
}

/**
 * Handles mouse move events within the scene and updates the drawing path.
 * This method extends the current path to the position of the mouse,
 * and updates the preview item if it exists, to reflect the new path.
 *
 * @param event The mouse event containing information about the mouse position.
 * @param scene The graphical scene in which the mouse event occurred.
 */
void LassoTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    path.lineTo(event->scenePos());
    if (preview)
        preview->setPath(path);
}

void LassoTool::mouseReleaseEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (!preview) return;

    path.closeSubpath();
    QPolygonF polygon = path.toFillPolygon();

    scene->removeItem(preview);
    delete preview;
    preview = nullptr;

    auto * area = new EffectPolygonItem(polygon, color, currentTextureName);
    dynamic_cast<MapScene*>(scene)->addUndoableItem(area);
}
