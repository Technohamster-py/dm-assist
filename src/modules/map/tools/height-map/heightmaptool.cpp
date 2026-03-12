#include "heightmaptool.h"

#include <QInputDialog>

/**
 * Constructs a HeightRegionItem with a specified polygon boundary and height value.
 * This item represents a polygonal region on the map with a specific height,
 * and is initialized with a translucent appearance and no border.
 *
 * @param poly The polygon that defines the boundary of the height region.
 * @param height The height value associated with this region.
 * @return A new instance of HeightRegionItem initialized with the given polygon and height.
 */
HeightRegionItem::HeightRegionItem(const QPolygonF &poly, qreal height) : QGraphicsPolygonItem(poly), m_height(height){
    setZValue(mapLayers::Height);
    setPen(Qt::NoPen);
    setOpacity(0.4);
    updateColor();
}

/**
 * Sets the height of the HeightRegionItem and updates its visual representation.
 * Adjusts the internal height property and recalculates the item's color based
 * on the new height value.
 *
 * @param h The new height value to be set for the region.
 */
void HeightRegionItem::setHeight(qreal h) {
    m_height = h;
    updateColor();
}

/**
 * Converts a specified height value into a corresponding QColor.
 * The height value is clamped between -100 and 100, and mapped using
 * a gradient ranging from blue for lower values to red for higher values.
 *
 * @param h The height value to convert, representing a numeric height.
 * @return A QColor representing the color associated with the given height.
 */
QColor HeightRegionItem::heightToColor(qreal h) {
    qreal  norm = qBound(-100.0, h, 100.0);
    qreal  t = (norm + 100) / 200.0;
    QColor low = QColor::fromRgb(0, 0, 255);
    QColor high = QColor::fromRgb(255, 0, 0);
    return QColor::fromRgbF(low.redF() * (1-t) + high.redF() * t,
                            low.greenF() * (1-t) + high.greenF() * t,
                            low.blueF() * (1-t) + high.blueF() * t);
}

/**
 * Updates the fill color of the HeightRegionItem based on its current height value.
 * This method converts the height value to a corresponding color using the heightToColor function
 * and then applies the resulting color as the brush for the polygonal region.
 */
void HeightRegionItem::updateColor() {
    setBrush(heightToColor(m_height));
}


/**
 * Handles mouse press events for the HeightMapTool. Depending on the mouse button pressed,
 * this method either removes an existing HeightRegionItem at the clicked position or initiates
 * the creation of a new path for a height map region preview.
 *
 * When the right mouse button is clicked, the method searches for a HeightRegionItem at the
 * clicked position, deletes it from the scene if found, and returns immediately.
 *
 * For other mouse buttons, it initializes a QPainterPath at the clicked position and creates
 * a new QGraphicsPathItem as a preview, configured with a dashed pen and appropriate Z-layer,
 * which is then added to the scene.
 *
 * @param event The mouse press event that contains information about the mouse state,
 *              such as position and button pressed.
 * @param scene The QGraphicsScene where the items are managed, modified, or added.
 */
void HeightMapTool::mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene * scene) {
    if (event->button() == Qt::RightButton) {
        HeightRegionItem* regionItem = dynamic_cast<HeightRegionItem*>(scene->itemAt(event->scenePos(), QTransform()));
        if (regionItem){
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
 * Handles the mouse release event when defining a height region on the map.
 *
 * This method is called when the mouse button is released to finalize the creation
 * of a height region in the map editing tool. If a preview item exists, it closes the
 * path, converts it to a polygon, and removes the preview item from the scene.
 * It then prompts the user to input a height value for the region. If the input
 * is valid, a new HeightRegionItem is created with the specified polygon and height
 * value, and the region is added to the scene as an undoable item.
 *
 * @param event The mouse event triggered by releasing the mouse button.
 * @param scene The graphics scene associated with the map tool, where the height
 *              region is being defined.
 */
void HeightMapTool::mouseReleaseEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (!preview) return;

    path.closeSubpath();
    QPolygonF polygon = path.toFillPolygon();

    scene->removeItem(preview);
    delete preview;
    preview = nullptr;

    bool ok;
    qreal h = QInputDialog::getDouble(nullptr, "Height", "Set region height (from -100.0 to 100.0", 0.0, -100.0, 100.0, 1, &ok);
    if (!ok) return;

    auto* region = new HeightRegionItem(polygon, h);

    dynamic_cast<MapScene*>(scene)->addUndoableItem(region);
}
