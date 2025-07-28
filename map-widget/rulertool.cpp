#include "rulertool.h"
#include "mapscene.h"

RulerTool::RulerTool(QObject *parent) : AbstractMapTool(parent) {}

/**
 * @brief Handles mouse press events for the RulerMapTool.
 *
 * This function processes mouse input to provide the functionality for measuring distances
 * on a map using a ruler tool. Left mouse button presses are used to define measurement points,
 * while right mouse button presses allow removal of existing graphics items, such as lines and labels.
 *
 * - If the left mouse button is pressed and no measurement point is currently defined, the position
 *   of the event is added to the toolPoints list.
 * - If the left mouse button is pressed and a previous measurement point exists, the function calculates
 *   the distance between the two points, scales it according to the scale factor of the MapScene,
 *   and displays a line and a label showing the distance in feet.
 * - Temporary graphics items (such as lines or labels used for live measurement) are cleaned up
 *   after finalizing a measurement.
 * - Right mouse button presses are used to remove graphics items, such as lines or labels, from the
 *   scene permanently.
 *
 * @param event The QGraphicsSceneMouseEvent that triggered the function.
 * @param scene The QGraphicsScene in which the mouse press event occurred.
 */
void RulerTool::mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    QPointF point = event->scenePos();

    if (event->button() == Qt::RightButton) {
        QGraphicsItem *item = scene->itemAt(point, QTransform());
        if (item && permanentItems.contains(item)) {
            QGraphicsItem* partner = (item->data(0).value<QGraphicsItem*>());
            if (partner){
                scene->removeItem(partner);
                permanentItems.removeOne(partner);
                delete partner;
            }
            scene->removeItem(item);
            permanentItems.removeOne(item);
            delete item;
        }
        return;
    }

    if (toolPoints.isEmpty()) {
        toolPoints.append(point);
    } else {
        toolPoints.append(point);
        QLineF line(toolPoints[0], toolPoints[1]);
        if (auto mapScene = qobject_cast<MapScene*>(scene)) {
            double distance = line.length() * mapScene->getScaleFactor();
            qreal dz = abs(mapScene->heightAt(toolPoints[0]) - mapScene->heightAt(toolPoints[1]));
            distance = std::sqrt(distance*distance + dz*dz);
            qreal width = mapScene->lineWidth();

            // Line
            auto lineItem = scene->addLine(line, QPen(QColor::fromRgb(252, 115, 3), width));
            lineItem->setZValue(mapLayers::Ruler);

            // Label
            auto label = scene->addText(QString("%1 ft").arg(distance, 0, 'f', 1));
            QFont font;
            font.setPointSizeF(width*4);
            label->setPos((line.p1() + line.p2()) / 2);
            label->setFont(font);
            label->setDefaultTextColor(Qt::red);
            label->setZValue(mapLayers::Ruler);


            lineItem->setData(0, QVariant::fromValue<QGraphicsItem*>(label));
            label->setData(0, QVariant::fromValue<QGraphicsItem*>(lineItem));

            permanentItems.append(lineItem);
            permanentItems.append(label);

            if (tempLine) scene->removeItem(tempLine);
            if (tempLabel) scene->removeItem(tempLabel);
            delete tempLine;
            delete tempLabel;
            tempLine = nullptr;
            tempLabel = nullptr;

            toolPoints.clear();
        }
    }
}

/**
 * @brief Handles the mouse move event to dynamically display a measuring line and distance label.
 *
 * This function is triggered when the mouse is moved while the tool is active.
 * If a single point has already been defined (toolPoints contains exactly one point),
 * it calculates the distance between the initially defined point and the current mouse position,
 * draws a temporary dashed line between these points, and displays the distance as a label.
 *
 * If the underlying scene is a `MapScene`, the calculated distance will respect the scene's
 * scaling factor. The temporary line and label are updated dynamically with each mouse movement.
 *
 * @param event The mouse event containing the current position of the mouse cursor in the scene.
 * @param scene The scene in which the ruler tool is being used. Must be of type `MapScene`
 *              or derived from `QGraphicsScene`.
 */
void RulerTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (toolPoints.size() == 1) {
        QPointF current = event->scenePos();
        QLineF line(toolPoints[0], current);
        if (auto mapScene = qobject_cast<MapScene*>(scene)) {
            double distance = line.length() * mapScene->getScaleFactor();
            qreal dz = abs(mapScene->heightAt(toolPoints[0]) - mapScene->heightAt(current));
            distance = std::sqrt(distance*distance + dz*dz);
            if (!tempLine) {
                tempLine = scene->addLine(line, QPen(static_cast<const QBrush>(nullptr), mapScene->lineWidth(), Qt::DashLine));
            } else {
                tempLine->setLine(line);
            }

            if (!tempLabel) {
                tempLabel = scene->addText(QString("%1 ft").arg(distance, 0, 'f', 1));
                tempLabel->setDefaultTextColor(Qt::gray);
                tempLabel->setTextWidth(400);
            } else {
                tempLabel->setPlainText(QString("%1 ft").arg(distance, 0, 'f', 1));
            }
            tempLabel->setPos((line.p1() + line.p2()) / 2);
        }
    }
}

/**
 * Deactivates the RulerMapTool, clearing temporary items and resources associated with it.
 *
 * This function performs the clean-up of the tool by:
 * - Clearing the list of points (`toolPoints`) that were previously used by the tool.
 * - Removing and deleting the temporary line item (`tempLine`) from the provided scene
 *   if it exists, and resetting the pointer to `nullptr`.
 * - Removing and deleting the temporary label item (`tempLabel`) from the provided scene
 *   if it exists, and resetting the pointer to `nullptr`.
 *
 * @param scene A pointer to the `QGraphicsScene` instance where the tool was active.
 *              This scene is used to remove temporary graphical elements.
 */
void RulerTool::deactivate(QGraphicsScene *scene) {
    toolPoints.clear();

    if (tempLine) {
        scene->removeItem(tempLine);
        delete tempLine;
        tempLine = nullptr;
    }
    if (tempLabel) {
        scene->removeItem(tempLabel);
        delete tempLabel;
        tempLabel = nullptr;
    }
}
