#include "spellshapetool.h"
#include "texturemanager.h"
#include "mapscene.h"

SpellShapeTool::SpellShapeTool(QObject *parent)
        : AbstractMapTool(parent) {
    color.setAlpha(180);
}

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
void SpellShapeTool::clearPreview(QGraphicsScene *scene) {
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
void SpellShapeTool::setColor(QColor c) {
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
bool SpellShapeTool::clearShapeAt(QGraphicsScene *scene, QPointF point) {
    QList<QGraphicsItem*> items = scene->items(point);

    for (QGraphicsItem* item : items) {
        if (!item) return false;

        if (item->zValue() < -99 || item->zValue() > 99) continue;


        dynamic_cast<MapScene *>(scene)->removeUndoableItem(item);
        return true;
    }
    return false;
}

/**
 * Handles the right-click event for the SpellShapeTool on a QGraphicsScene.
 *
 * This method processes a right-click mouse event and performs specific actions
 * based on the state of the SpellShapeTool. If the tool has a first point initialized
 * (`hasFirstPoint == true`) and the mouse click corresponds to the right button,
 * it clears the shape at the clicked position, disables the first point, and removes
 * any preview shapes or labels from the scene.
 *
 * @param event The mouse event triggered by the user's right-click.
 * @param scene The graphics scene on which the mouse event occurred.
 */
void SpellShapeTool::rightClickEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (hasFirstPoint){
        if (event->button() == Qt::RightButton) {
            clearShapeAt(scene, event->scenePos());
            hasFirstPoint = false;
            clearPreview(scene);
            return;
        }
    }
}

QBrush SpellShapeTool::getBrush() {
    auto pixmap = TextureManager::instance().getTexture(currentTextureName);
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


/**
 * @brief Handles mouse press events to interact with the scene for drawing or clearing line shapes.
 *
 * This method is used to initiate or complete the drawing of a line shape in the provided
 * QGraphicsScene based on the user's mouse interaction. It supports the following behaviors:
 * - If the first point hasn't been set:
 *   - Right mouse button clears any shape at the click position and exits.
 *   - Otherwise, the first point of the line is set, and further interaction is awaited.
 * - If the first point has been set:
 *   - Right mouse button cancels the line creation process, resets the state,
 *     and clears any preview.
 *   - Left mouse button completes the line creation process by:
 *     - Adding a new, stylized line to the scene.
 *     - Registering the item as undoable in the scene.
 *     - Resetting the tool's state.
 *     - Clearing any previewed line.
 *
 * @param event A pointer to the QGraphicsSceneMouseEvent containing details of the mouse interaction.
 * @param scene A pointer to the QGraphicsScene in which the interaction occurs.
 *
 * The `hasFirstPoint` member variable tracks whether the first point of the line has been set.
 * The `color` member variable is used to define the color of the newly created line.
 * Helper methods `clearShapeAt` and `clearPreview` are used to manage scene state during the interaction process.
 */
void LineShapeTool::mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (!hasFirstPoint){
        if (event->button() == Qt::RightButton) {
            clearShapeAt(scene, event->scenePos());
            return;
        }

        firstPoint = event->scenePos();
        hasFirstPoint = true;
    } else {
        if (event->button() == Qt::RightButton){
            hasFirstPoint = false;
            clearPreview(scene);
            return;
        }

        QLineF line(firstPoint, event->scenePos());

        auto item = new QGraphicsLineItem(line);
        item->setPen(QPen(color, 2));
        item->setZValue(mapLayers::Shapes);

        dynamic_cast<MapScene*>(scene)->addUndoableItem(item);

        hasFirstPoint = false;
        clearPreview(scene);
    }
}

/**
 * @brief Handles the mouse move event to create a preview of a line and displays its length dynamically.
 *
 * This function is called during a mouse move event. It manages the preview of a line shape
 * based on the current mouse position, calculates the distance of the line, and updates a label
 * with that distance. The behavior adapts depending on the type of the provided QGraphicsScene.
 *
 * @param event The mouse event that contains the current cursor position in the scene.
 * @param scene The QGraphicsScene where the event is occurring. This can optionally be a MapScene to use map-specific scaling.
 *
 * If the first point has not been set (`hasFirstPoint` is false), the function returns immediately.
 * Otherwise, it draws a dashed line preview from the `firstPoint` to the current mouse position.
 * If `scene` is a MapScene, the measured distance is scaled using the scene's scale factor. The label
 * is positioned near the midpoint of the line and displays the distance in feet.
 *
 * - If `previewShape` is nullptr, a new line item is created and added to the scene. Otherwise,
 *   the existing line item is updated.
 * - If `previewLabel` is nullptr, a new text item is created and added to the scene. Otherwise,
 *   the existing label is updated with the recalculated distance.
 */
void LineShapeTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (!hasFirstPoint)
        return;

    QLineF line(firstPoint, event->scenePos());
    if (!previewShape) {
        previewShape = scene->addLine(line, QPen(Qt::DashLine));
    } else {
        dynamic_cast<QGraphicsLineItem*>(previewShape)->setLine(line);
    }

    double distance = 0.0;

    if (auto mapScene = qobject_cast<MapScene*>(scene)){
        distance = line.length() * mapScene->getScaleFactor();
    } else {
        distance = line.length();
    }

    if (!previewLabel) {
        previewLabel = scene->addText(QString("%1 ft").arg(distance, 0, 'f', 1));
    } else {
        previewLabel->setPlainText(QString("%1 ft").arg(distance, 0, 'f', 1));
    }
    previewLabel->setPos(line.pointAt(0.5) + QPointF(10, -10));
}

/**
 * @brief Handles the mouse press event for the CircleShapeTool.
 *
 * This function processes mouse press events to define and create circular shapes:
 * - On the first left mouse button press, it sets the initial point for the circle.
 * - On the second left mouse button press, it calculates the circle's radius from the initial point
 *   and creates a corresponding QGraphicsEllipseItem added to the scene as an undoable action.
 * - If the right mouse button is pressed during the first or second phase,
 *   it either clears the preview shape or removes an existing shape at the clicked position.
 *
 * @param event The mouse press event containing information about the mouse interaction.
 * @param scene The graphics scene where the shapes are being drawn and managed.
 */
void CircleShapeTool::mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (!hasFirstPoint) {
        if (event->button() == Qt::RightButton) {
            clearShapeAt(scene, event->scenePos());
            return;
        }
        firstPoint = event->scenePos();
        hasFirstPoint = true;
    } else {
        if (event->button() == Qt::RightButton){
            hasFirstPoint = false;
            clearPreview(scene);
            return;
        }

        QRectF rect = circleRect(event->scenePos());

        auto item = new QGraphicsEllipseItem(rect.normalized());
        item->setPen(QPen(color));
        item->setBrush(getBrush());
        item->setZValue(mapLayers::Shapes);

        dynamic_cast<MapScene*>(scene)->addUndoableItem(item);

        hasFirstPoint = false;
        clearPreview(scene);
    }
}

/**
 * Handles the mouse move event for the CircleShapeTool.
 *
 * This method is responsible for previewing the circle shape and its radius label
 * based on the current mouse position during a drag operation. It updates the
 * temporary circle preview (dashed circle) as well as the radius label near
 * the center of the circle.
 *
 * If the first point of the circle has not been set, the method immediately exits.
 * Otherwise:
 *  - It calculates the rectangular boundary of the circle based on the mouse position.
 *  - If no preview shape exists, it creates a new dashed circle for preview;
 *    otherwise, it updates the existing preview shape's size and position.
 *  - It calculates the radius of the circle, and if no radius label exists, it
 *    creates one near the circle's center. The label is updated with the radius
 *    every time the mouse moves.
 *
 * @param event The mouse event containing the current position within the scene.
 * @param scene The graphics scene where the circle and label preview are being added.
 */
void CircleShapeTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (!hasFirstPoint) return;

    QRectF rect = circleRect(event->scenePos());
    if (!previewShape) {
        previewShape = scene->addEllipse(rect.normalized(), QPen(Qt::DashLine));
    } else {
        dynamic_cast<QGraphicsEllipseItem*>(previewShape)->setRect(rect.normalized());
    }

    double radius = QLineF(firstPoint, event->scenePos()).length();
    if (!previewLabel) {
        previewLabel = scene->addText(QString::number(radius, 'f', 1) + " ft");
    }
    previewLabel->setPos(rect.center() + QPointF(10, -10));
}

/**
 * Calculates a rectangular boundary for a circle based on a given point.
 *
 * This function computes the rect defining a circle's bounds using the distance
 * between the `firstPoint` and the provided `point` as the radius. The circle
 * is centered at `firstPoint`.
 *
 * @param point The point used to determine the circle's radius.
 * @return A QRectF object defining the rectangular boundary of the circle.
 */
QRectF CircleShapeTool::circleRect(QPointF point) {
    double radius = QLineF(firstPoint, point).length();

    QPointF topLeft(firstPoint.x()-radius, firstPoint.y()-radius);
    QPointF bottomRight(firstPoint.x()+radius, firstPoint.y()+radius);

    return {topLeft, bottomRight};
}

/**
 * @brief Handles mouse press events for creating or modifying a triangular shape in the scene.
 *
 * This method allows for the creation of a triangular shape on a QGraphicsScene. The triangle
 * is defined by two points: the first point is set upon the initial mouse press, and the triangle
 * is finalized on the second press. If the right mouse button is pressed instead, the behavior
 * depends on the current state:
 * - If no first point is set, it clears any existing shape at the clicked position.
 * - If the first point is already set, the preview of the shape is cleared, and the tool resets.
 *
 * The triangle is constructed using the current point and the previously stored first point.
 * The shape is added to the scene as an undoable item, allowing for future undo actions, and
 * visualized with a specified pen and brush pattern.
 *
 * @param event The mouse event containing the properties of the press, such as position and button used.
 * @param scene The QGraphicsScene where the triangle is being drawn or modified.
 */
void TriangleShapeTool::mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (!hasFirstPoint) {
        if (event->button() == Qt::RightButton) {
            clearShapeAt(scene, event->scenePos());
            return;
        }

        firstPoint = event->scenePos();
        hasFirstPoint = true;
    } else {
        if (event->button() == Qt::RightButton){
            hasFirstPoint = false;
            clearPreview(scene);
            return;
        }

        QPolygonF triangle = buildTriangle(event->scenePos());

        auto item = new QGraphicsPolygonItem(triangle);
        item->setPen(QPen(color));
        item->setBrush(getBrush());
        item->setZValue(mapLayers::Shapes);

        dynamic_cast<MapScene*>(scene)->addUndoableItem(item);

        hasFirstPoint = false;
        clearPreview(scene);
    }
}

/**
 * Handles the mouse move event for dynamically updating a triangular shape preview
 * based on the current cursor position in the scene.
 *
 * This method updates the triangle and its associated base length label as the
 * user drags the mouse, provided that the first point of the triangle is already
 * set. It performs the following operations:
 *
 * - Updates or creates a preview triangle, with its vertices dynamically adjusted
 *   for the current mouse position.
 * - Calculates the base length of the triangle and displays it as a text label.
 * - Positions the label near the center of the triangle's base, slightly offset.
 *
 * Prerequisites:
 * - The `hasFirstPoint` flag must be set to true, indicating that the first point
 *   of the triangle has been established.
 *
 * Behavior:
 * - If this is the first time moving the mouse after setting the first point, a new
 *   preview triangle is created in the scene. Subsequent movements update the existing
 *   triangle's points.
 * - The base length of the triangle is calculated and displayed in feet, with single-decimal
 *   precision.
 *
 * Parameters:
 * - event: The mouse event containing the current cursor position in the scene.
 * - scene: The graphics scene where the preview triangle and label are being rendered.
 */
void TriangleShapeTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (!hasFirstPoint) return;

    QPointF p2 = event->scenePos();
    QPolygonF triangle = buildTriangle(p2);

    if (!previewShape) {
        previewShape = scene->addPolygon(triangle, QPen(Qt::DashLine));
    } else {
        dynamic_cast<QGraphicsPolygonItem*>(previewShape)->setPolygon(triangle);
    }

    double baseLength = QLineF(firstPoint, p2).length();
    if (!previewLabel) {
        previewLabel = scene->addText(QString::number(baseLength, 'f', 1) + " ft");
    }
    QPointF center = (firstPoint + p2) / 2;
    previewLabel->setPos(center + QPointF(10, -10));
}

/**
 * @brief Constructs a triangle given its height point and the first vertex.
 *
 * This method calculates and returns a triangle shape defined by one vertex
 * (`firstPoint`) and a height point (`hPoint`). The triangle is constructed
 * by finding the base perpendicular to the vector formed between `hPoint` and
 * `firstPoint`, and using this base to determine the other two vertices of the triangle.
 *
 * @param hPoint The height point of the triangle, opposite to the base.
 * @return A QPolygonF representing the constructed triangle.
 */
QPolygonF TriangleShapeTool::buildTriangle(const QPointF &hPoint) {
    QPointF a = firstPoint;
    QPointF h = hPoint;

    QLineF ah(h, a);
    double height = ah.length();

    QLineF baseLine = ah.normalVector();
    baseLine.setLength(height);
    QPointF offset = (baseLine.p2() - baseLine.p1()) / 2;

    QPointF b = h + offset;
    QPointF c = h - offset;

    QPolygonF triangle; triangle << a << b << c;
    return triangle;
}


/**
 * @brief Handles mouse press events to create or remove square shapes in the scene.
 *
 * This function is responsible for processing user interactions within a specific
 * graphical scene. If the user presses the left mouse button, the function either:
 * - Stores the first point for a new square (if no first point has been defined yet).
 * - Creates a square shape using the previously stored first point and the current
 *   mouse position. The square is added to the scene as an undoable action.
 *
 * If the user presses the right mouse button, the function attempts to clear an
 * existing shape at the mouse position using the `clearShapeAt` method.
 *
 * The created square is styled with a pen and brush of the predefined `color`, and
 * its z-index is set to 5.
 *
 * This function also clears any preview elements from the scene when a square is created
 * or removed.
 *
 * @param event A pointer to the QGraphicsSceneMouseEvent containing details of the mouse event.
 * @param scene A pointer to the QGraphicsScene where the operation is performed.
 */
void SquareShapeTool::mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (!hasFirstPoint) {
        if (event->button() == Qt::RightButton) {
            clearShapeAt(scene, event->scenePos());
            return;
        }

        firstPoint = event->scenePos();
        hasFirstPoint = true;
    } else {
        if (event->button() == Qt::RightButton) {
            clearShapeAt(scene, event->scenePos());
            return;
        }

        QPolygonF square = buildSquare(firstPoint, event->scenePos());

        auto item = new QGraphicsPolygonItem(square);
        item->setPen(QPen(color));
        item->setBrush(getBrush());
        item->setZValue(mapLayers::Shapes);

        dynamic_cast<MapScene*>(scene)->addUndoableItem(item);
        hasFirstPoint = false;
        clearPreview(scene);
    }
}

/**
 * @brief Handles mouse move events to update the preview square and accompanying label.
 *
 * Updates a square preview shape and a text label indicating the diagonal length
 * in response to the user's mouse movement. If the first point has been set, it draws
 * or modifies a square shape connecting the initial and current mouse positions.
 *
 * - The square is calculated using the `buildSquare` method which ensures the shape
 *   remains a perfect square aligned with the given points.
 * - The preview shape is drawn with a dashed line.
 * - A text label is displayed, showing the diagonal length in feet, formatted to one decimal place.
 *
 * @param event Pointer to the QGraphicsSceneMouseEvent containing mouse event data.
 * @param scene Pointer to the scene where the shapes are being drawn.
 *
 * If the first point (`hasFirstPoint`) is not set, the function performs no action.
 * Otherwise:
 * - It calculates a new square using `buildSquare` with the initial point and the current mouse position.
 * - If the preview shape does not exist, it creates a new QGraphicsPolygonItem to display the square.
 *   If the preview shape exists, it updates the square polygon.
 * - The diagonal length of the square is calculated and displayed as a label in the scene.
 * - The label is positioned near the center of the square, offset by a small vector.
 */
void SquareShapeTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (!hasFirstPoint) return;

    QPolygonF square = buildSquare(firstPoint, event->scenePos());
    if (!previewShape) {
        previewShape = scene->addPolygon(square, QPen(Qt::DashLine));
    } else {
        dynamic_cast<QGraphicsPolygonItem*>(previewShape)->setPolygon(square);
    }

    double diagLength = QLineF(firstPoint, event->scenePos()).length();
    if (!previewLabel) {
        previewLabel = scene->addText(QString::number(diagLength, 'f', 1) + " ft");
    }
    QPointF center = (firstPoint + event->scenePos()) / 2;
    previewLabel->setPos(center + QPointF(10, -10));
}

/**
 * @brief Constructs a square polygon based on the given diagonal endpoints.
 *
 * This function takes two points representing the diagonal endpoints of a square
 * and calculates the vertices of the square. It returns the constructed square
 * as a QPolygonF object.
 *
 * @param p1 The first endpoint of the square's diagonal.
 * @param p2 The second endpoint of the square's diagonal.
 * @return A QPolygonF object representing the constructed square, with its
 *         vertices in counterclockwise order starting from one corner.
 */
QPolygonF SquareShapeTool::buildSquare(const QPointF &p1, const QPointF &p2) {
    QLineF diag(p1, p2);
    QPointF center = p1 + (p2 - p1) * 0.5;

    QLineF side1 = diag.normalVector(); side1.setLength(diag.length());
    QPointF v1 = side1.p2() - side1.p1();

    QLineF side2 = diag; side2.setLength(diag.length());
    QPointF v2 = side2.p2() - side2.p1();

    QPointF a = center - v1 / 2 - v2 / 2;
    QPointF b = center + v1 / 2 - v2 / 2;
    QPointF c = center + v1 / 2 + v2 / 2;
    QPointF d = center - v1 / 2 + v2 / 2;

    QPolygonF square; square << a << b << c << d;
    return square;
}

