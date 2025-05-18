#include "calibrationtool.h"
#include "mapscene.h"
#include <QInputDialog>
#include <QLineF>
#include <QGraphicsSceneMouseEvent>
#include <QPen>

CalibrationTool::CalibrationTool(QObject *parent)
        : AbstractMapTool(parent) {}

/**
 * @brief Handles mouse press event for the CalibrationTool.
 *
 * This function responds to a left mouse button press event. It captures the
 * position of the mouse click, and if two consecutive clicks are detected, it:
 * - Calculates the distance in pixels between the two points.
 * - Prompts the user to input a corresponding real-world distance in feet
 *   through a dialog.
 * - Calculates and sets the scale factor for the map scene if the user input
 *   is valid.
 *
 * If a preview line exists, it removes it from the scene and clears the
 * temporary point data after the calculation. The function also ensures the
 * operation emits a `finished()` signal upon completion.
 *
 * @param event Pointer to the QGraphicsSceneMouseEvent which contains details
 *        about the mouse interaction in the scene.
 * @param scene Pointer to the QGraphicsScene on which the mouse event occurred.
 */
void CalibrationTool::mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    QPointF pos = event->scenePos();

    if (event->button() != Qt::LeftButton)
        return;

    points.append(pos);

    if (points.size() == 2) {
        QLineF line(points[0], points[1]);
        double pixelDistance = line.length();

        bool ok;
        double realDistance = QInputDialog::getDouble(nullptr, tr("Scale calibration"),
                                                      tr("Enter distance in feet"),
                                                      5.0, 0.01, 10000, 2, &ok);
        if (ok && pixelDistance > 0.0) {
            double scale = realDistance / pixelDistance;
            auto mapScene = dynamic_cast<MapScene*>(scene);
            if (mapScene) {
                mapScene->setScaleFactor(scale);
            }
        }

        if (previewLine) {
            scene->removeItem(previewLine);
            delete previewLine;
            previewLine = nullptr;
        }

        points.clear();
        emit finished();
    }
}

/**
 * Handles mouse movement within the scene for the calibration tool.
 *
 * This function is triggered when a mouse move event occurs within the scene.
 * It updates the temporary preview line displayed during the point selection process.
 *
 * Behavior:
 * - If there is exactly one point already stored in `points`:
 *   - Calculates a line from the first point to the current mouse position.
 *   - If `previewLine` does not already exist, it adds a new dashed preview line to the scene.
 *   - Otherwise, updates the existing `previewLine` to match the new line geometry.
 *
 * @param event The QGraphicsSceneMouseEvent that provides information about the mouse event,
 *              including the current mouse position.
 * @param scene The QGraphicsScene where the calibration actions are taking place. Used to
 *              add or modify the preview line in the scene.
 */
void CalibrationTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (points.size() == 1) {
        QPointF current = event->scenePos();
        QLineF line(points[0], current);

        if (!previewLine) {
            previewLine = scene->addLine(line, QPen(Qt::DashLine));
        } else {
            previewLine->setLine(line);
        }
    }
}

/**
 * Deactivates the calibration tool, clearing its state and removing the preview line from the scene.
 *
 * This function is responsible for cleaning up any intermediate data or visual components
 * created during the use of the calibration tool. Specifically:
 * - Clears the list of points stored in the tool.
 * - Removes the preview line from the specified scene if it exists and deletes it
 *   to free the associated memory.
 *
 * @param scene The QGraphicsScene from which the preview line will be removed.
 */
void CalibrationTool::deactivate(QGraphicsScene *scene) {
    points.clear();
    if (previewLine) {
        scene->removeItem(previewLine);
        delete previewLine;
        previewLine = nullptr;
    }
}