/**
 * @file mapscene.cpp
 * @brief Implementation of MapScene class.
 */

#include "mapscene.h"
#include "lighttool.h"
#include "heightmaptool.h"
#include "tokenitem.h"
#include <QBuffer>
#include <QFile>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPainter>
#include <QKeyEvent>
#include <QMimeData>

#include <QDebug>

/**
 * @brief Constructs a MapScene object with the specified parent and initializes its background.
 *
 * This constructor initializes the MapScene by calling the QGraphicsScene constructor
 * with the parent QObject. It also sets a dark gray background brush.
 *
 * @param parent The parent QObject of the scene, which can be nullptr if no parent is specified.
 */
MapScene::MapScene(QObject *parent)
        : QGraphicsScene(parent)
{
    setBackgroundBrush(Qt::darkGray);

}

/**
 * @brief Handles mouse press events within the scene, delegating to the active tool if available.
 *
 * This method intercepts and processes mouse press events in the QGraphicsScene.
 * When an active tool is set, the event is forwarded to the tool's implementation
 * of mousePressEvent, along with a reference to the MapScene. If no active tool
 * is specified, the default QGraphicsScene behavior is executed.
 *
 * @param event The mouse press event to be handled.
 */
void MapScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (m_activeTool) {
        m_activeTool->mousePressEvent(event, this);
    } else {
        QGraphicsScene::mousePressEvent(event);
    }
}


void MapScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    if (m_activeTool) {
        m_activeTool->mouseDoubleClickEvent(event, this);
    } else {
        auto* region = dynamic_cast<HeightRegionItem*>(itemAt(event->scenePos(), QTransform()));
        if (!region)
            return;

        bool ok;
        qreal h = QInputDialog::getDouble(nullptr, tr("Change height"), tr("NewHeight"), region->height(), -100, 100, 1, &ok);
        if (ok)
            region->setHeight(h);

        QGraphicsScene::mouseDoubleClickEvent(event);
    }
}

/**
 * @brief Handles mouse move events for the MapScene.
 *
 * This function intercepts mouse move events occurring within the MapScene.
 * If an active tool is set via m_activeTool, it delegates the handling of the
 * event to the tool by calling its mouseMoveEvent. Otherwise, it falls back
 * to the default behavior of QGraphicsScene's mouseMoveEvent.
 *
 * @param event Pointer to the QGraphicsSceneMouseEvent that provides details
 *        of the mouse move event.
 */
void MapScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (m_activeTool) {
        m_activeTool->mouseMoveEvent(event, this);
    } else {
        QGraphicsScene::mouseMoveEvent(event);
    }
}


/**
 * @brief Handles mouse release events within the MapScene.
 *
 * This method determines how the scene responds to a mouse release event based on the active tool.
 * If a tool is active, the mouse release event is delegated to the tool for handling specific behavior.
 * Otherwise, the default QGraphicsScene implementation processes the event.
 *
 * @param event Pointer to the QGraphicsSceneMouseEvent containing event data.
 */
void MapScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (m_activeTool){
        m_activeTool->mouseReleaseEvent(event, this);
    } else {
        QGraphicsScene::mouseReleaseEvent(event);
    }

}

/**
 * @brief Handles the wheel event for the map scene.
 *
 * This function processes wheel events to provide zooming functionality
 * when the Ctrl key is held. If a tool is currently active, the tool's
 * wheelEvent handler is called. Otherwise, the default QGraphicsScene's
 * wheelEvent handler is executed. If the Ctrl modifier is not held, the
 * event is passed directly to the base class implementation.
 *
 * @param event The QGraphicsSceneWheelEvent to be processed.
 */
void MapScene::wheelEvent(QGraphicsSceneWheelEvent *event) {
    if (!(event->modifiers() & Qt::ControlModifier)) {
        QGraphicsScene::wheelEvent(event);
        return;
    }
    if (m_activeTool)
        m_activeTool->wheelEvent(event, this);
    else
        QGraphicsScene::wheelEvent(event);
}

void MapScene::setScaleFactor(double factor) {
    m_scaleFactor = factor;
    if (m_gridItem)
        m_gridItem->setPixelsPerFoot(1/m_scaleFactor);

    emit scaleChanged(m_scaleFactor);
}

/**
 * @brief Sets the active map tool, deactivating the current tool if present.
 *
 * This method changes the currently active tool for the MapScene. If a tool is
 * already active, it calls its `deactivate` method with the current scene before
 * switching to the new tool. Once the new tool is set as active, the
 * `toolChanged` signal is emitted.
 *
 * @param tool A pointer to the new map tool to be set as active. Can be nullptr
 *             if no tool is to be activated.
 *
 * @note The `deactivate` method of the previously active tool is called, if any,
 *       before switching to the new tool.
 * @note The `toolChanged` signal is emitted after successfully updating the
 *       active tool.
 */
void MapScene::setActiveTool(AbstractMapTool *tool) {
    if (m_activeTool)
        m_activeTool->deactivate(this);
    m_activeTool = tool;
    emit toolChanged(tool);
}

/**
 * @brief Initializes the fog of war in the map scene with the specified size.
 *
 * This function creates a new QImage filled with a transparent color to represent
 * the initial state of the fog (completely visible). If a fog item already exists,
 * it is removed from the scene and deleted. A new QGraphicsPixmapItem is then created
 * to display the fog image, added to the scene, and configured with the appropriate
 * layering and default opacity.
 *
 * @param size The dimensions of the fog image to be initialized.
 *
 * The fog item's z-value is set to 100 to ensure it is rendered above the map.
 * The default opacity of the fog is set to 0.5.
 */
void MapScene::initializeFog(const QSize &size) {
    fogImage = QImage(size, QImage::Format_ARGB32_Premultiplied);
    fogImage.fill(Qt::transparent);
}

/**
 * @brief Modifies the fog of war by drawing a circular area based on the given parameters.
 *
 * This function manipulates the fog of war image (`fogImage`) to create or remove fog
 * within a circular region defined by a center point, radius, and visibility flag. It
 * uses a QPainter to render the changes on the `fogImage`. If the `hide` parameter is
 * true, the circle will be drawn with black (indicating fog). If false, the circle will
 * be made transparent (indicating visibility).
 *
 * After modifying the fog image, the function updates the fog display by setting the
 * modified `fogImage` on the `fogItem`, if it exists, and emits the `fogUpdated` signal
 * to notify other components of the change.
 *
 * @param scenePos The center point of the circle, in scene coordinates.
 * @param radius The radius of the circle to be drawn, in pixels.
 * @param hide A boolean indicating whether the fog should be hidden (black) or revealed (transparent).
 */
void MapScene::drawFogCircle(const QPointF &scenePos, int radius, bool hide) {
    QPainter painter(&fogImage);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPoint center = scenePos.toPoint();
    QBrush brush(hide ? Qt::black : Qt::transparent);
    QPen pen(Qt::NoPen);
    painter.setCompositionMode(hide ? QPainter::CompositionMode_SourceOver
                                    : QPainter::CompositionMode_Clear);
    painter.setBrush(brush);
    painter.setPen(pen);
    painter.drawEllipse(center, radius, radius);
    painter.end();

    emit fogUpdated(fogImage);
    update();
}

/**
 * @brief Retrieves the pixmap of the first QGraphicsPixmapItem in the scene, excluding the fog item.
 *
 * This function iterates over all items in the scene in ascending Z-order to find the first
 * QGraphicsPixmapItem that is not the fog item. If such an item is found, its pixmap is returned.
 * If no suitable QGraphicsPixmapItem is found, an empty QPixmap is returned.
 *
 * @return The QPixmap of the matching QGraphicsPixmapItem, or an empty QPixmap if no match is found.
 */
QPixmap MapScene::getMapPixmap() const {
    auto mapItems = items(Qt::AscendingOrder);
    for (auto *item : mapItems) {
        if (auto pixmapItem = qgraphicsitem_cast<QGraphicsPixmapItem *>(item)) {
            return pixmapItem->pixmap();
        }
    }
    return {};
}

/**
 * @brief Draws a path on the fog layer.
 *
 * This method is responsible for modifying the fog of war by either hiding or revealing parts of it.
 * The drawing is performed using a given QPainterPath and the hide parameter determines whether the
 * path is filled with black (to hide) or cleared (to reveal).
 *
 * The method takes advantage of QPainter's rendering capabilities and adjusts the fogImage
 * accordingly. Once the drawing operation is complete, it triggers an update to the fog by calling
 * the updateFog method, which synchronizes the visual representation of the fog.
 *
 * @param path The QPainterPath defining the shape to be drawn on the fog.
 * @param hide Boolean flag indicating whether to hide (true) or reveal (false) the fog along the specified path.
 */
void MapScene::drawFogPath(const QPainterPath &path, bool hide) {
    QPainter painter(&fogImage);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(hide ? Qt::black : Qt::transparent);
    painter.setCompositionMode(hide ? QPainter::CompositionMode_SourceOver
                                    : QPainter::CompositionMode_Clear);
    painter.drawPath(path);
    painter.end();

    update();
}

/**
 * @brief Clears the fog of war by resetting the fog image and updating the scene.
 *
 * This method sets the fog image to completely transparent using `Qt::transparent`,
 * effectively clearing all fog from the map. After modifying the fog image,
 * it invokes the `updateFog()` method to refresh the visual representation
 * of the fog and notify any listeners about the update.
 *
 * @note This operation modifies the internal `fogImage` and emits
 * a `fogUpdated()` signal to reflect the changes.
 */
void MapScene::clearFog(bool clear) {
    if (clear)
        fogImage.fill(Qt::transparent);
    else
        fogImage.fill(Qt::black);

    update();
}

/**
 * @brief Draws a scaled circular region on the scene to manipulate the fog of war.
 *
 * This method calculates the actual radius of the circle based on the current scaling factor
 * and delegates the task of drawing the circle to the drawFogCircle method. The circle's effect
 * can either reveal or conceal parts of the scene depending on the `hide` parameter.
 *
 * @param scenePos A QPointF representing the center position of the circle in scene coordinates.
 * @param radius An integer value specifying the radius of the circle in unscaled units.
 * @param hide A boolean indicating whether to conceal (true) or reveal (false) within the circle.
 */
void MapScene::drawScaledCircle(const QPointF &scenePos, int radius, bool hide) {
    int realRadius = static_cast<int>(radius / m_scaleFactor);
    drawFogCircle(scenePos, realRadius, hide);
}

/**
 * @brief Handles key press events in the MapScene.
 *
 * This function intercepts key presses and checks if the user input matches a predefined
 * key sequence for undoing the last action (QKeySequence::Undo). If so, it invokes the
 * undoLastAction() method, performing an undo operation for the last scene change. If the
 * input does not match the undo sequence, the event is passed to the base QGraphicsScene's
 * keyPressEvent for default handling.
 *
 * @param event Pointer to the QKeyEvent object representing the key press event.
 */
void MapScene::keyPressEvent(QKeyEvent *event) {
    if (event->matches(QKeySequence::Undo)) {
        undoLastAction();
        return;
    }
    QGraphicsScene::keyPressEvent(event);
}

/**
 * @brief Adds a QGraphicsItem to the scene and registers the operation in the undo stack.
 *
 * This method adds the provided QGraphicsItem to the MapScene and simultaneously
 * creates an AddItemAction, which is pushed onto the undo stack. This enables the
 * addition of the item to be undone later via the undo mechanism of the scene.
 *
 * @param item The QGraphicsItem to be added to the scene.
 */
void MapScene::addUndoableItem(QGraphicsItem *item) {
    addItem(item);
    undoStack.push(std::make_unique<AddItemAction>(item));
}

/**
 * @brief Removes a QGraphicsItem from the scene with undo/redo capability.
 *
 * This function removes the specified QGraphicsItem from the MapScene while
 * enabling the operation to be reversible by pushing a corresponding
 * RemoveItemAction onto the undo stack. The RemoveItemAction is created to
 * manage the undo functionality, ensuring that the item can be re-added
 * to the scene if undone.
 *
 * @param item A pointer to the QGraphicsItem to remove from the scene.
 */
void MapScene::removeUndoableItem(QGraphicsItem *item) {
    undoStack.push(std::make_unique<RemoveItemAction>(item));
    removeItem(item);
}

void MapScene::undoLastAction() {
    undoStack.undo(this);
}


/**
 * @brief Serializes the MapScene instance into a QJsonObject representation.
 *
 * This function creates a QJsonObject containing the current state of the MapScene, including:
 * - The scale factor used in the scene.
 * - Tools and their deactivation state.
 * - A list of all items present in the scene with their properties, types, and geometries.
 * - Fog of war data encoded as a PNG in Base64 format, if one exists.
 *
 * The following types of items are handled during serialization:
 * - Polygons: The points, color, and z-value are saved.
 * - Ellipses: The center, radius, and color are saved (assumes ellipses are circles).
 * - Lines: The start and end coordinates, as well as their color, are saved.
 * - Light sources: The center position, inner and outer radii, and light color are saved.
 *
 * If an item does not match any of these types, it is ignored.
 *
 * If the fogImage is available and not null, it is encoded and added to the JSON representation.
 *
 * @return A QJsonObject containing the serialized state of the MapScene.
 */
QJsonObject MapScene::toJson() {
    QJsonObject obj;
    obj["scaleFactor"] = m_scaleFactor;
    obj["gridCellSie"] = m_gridSize;
    obj["grid"] = m_gridType;

    if (m_activeTool)
        m_activeTool->deactivate(this);


    int currentProgress, step;
    if (items().count() > 0)
        step = std::floor((80 - 12) / items().count());
    currentProgress = 12;

    QJsonArray itemsArray;
    for (auto item : items()) {
        emit progressChanged(currentProgress+=step, "Collecting graphic objects");
        QJsonObject itemObj;

        if (auto* light = dynamic_cast<LightSourceItem*>(item)) {
            itemObj["type"] = "light";
            itemObj["center"] = QJsonArray{ light->pos().x(), light->pos().y() };
            itemObj["r1"] = light->brightRadius();
            itemObj["r2"] = light->dimRadius();
            itemObj["color"] = light->color().name();
        }
        else if (auto* heightRegion = dynamic_cast<HeightRegionItem*>(item)){
            itemObj["type"] = "heightRegion";
            QJsonArray points;
            for (const QPointF& p : heightRegion->polygon())
                points.append(QJsonArray{ p.x(), p.y() });
            itemObj["points"] = points;
            itemObj["height"] = heightRegion->height();
        }
        else if (auto* polyItem = qgraphicsitem_cast<QGraphicsPolygonItem*>(item)) {
            itemObj["type"] = "polygon";

            QJsonArray points;
            for (const QPointF& p : polyItem->polygon())
                points.append(QJsonArray{ p.x(), p.y() });
            itemObj["points"] = points;

            itemObj["color"] = polyItem->pen().color().name();
            itemObj["z"] = polyItem->zValue();
        }
        else if (auto* ellipse = qgraphicsitem_cast<QGraphicsEllipseItem*>(item)) {
            itemObj["type"] = "ellipse";
            QRectF r = ellipse->rect();
            itemObj["center"] = QJsonArray{ r.center().x(), r.center().y() };
            itemObj["radius"] = r.width() / 2; // предполагаем круг
            itemObj["color"] = ellipse->pen().color().name();
            itemObj["z"] = ellipse->zValue();

        }
        else if (auto* line = qgraphicsitem_cast<QGraphicsLineItem*>(item)) {
            itemObj["type"] = "line";
            QLineF l = line->line();
            itemObj["start"] = QJsonArray{ l.p1().x(), l.p1().y() };
            itemObj["end"]   = QJsonArray{ l.p2().x(), l.p2().y() };
            itemObj["color"] = line->pen().color().name();
            itemObj["z"] = line->zValue();

        }

        if (!itemObj.isEmpty())
            itemsArray.append(itemObj);
    }

    obj["items"] = itemsArray;


    if (!fogImage.isNull()) {
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        buffer.open(QIODevice::WriteOnly);
        fogImage.save(&buffer, "PNG");
        obj["fog"] = QString::fromLatin1(byteArray.toBase64());
    }
    return obj;
}


/**
 * @brief Populates the MapScene from a JSON object.
 *
 * This method parses a QJsonObject to populate the MapScene with various graphical items
 * (e.g., polygons, ellipses, lines, light sources) and updates settings like the scale factor.
 * It also supports loading a fog of war image from the JSON object if present.
 *
 * The method expects the following structure in the JSON object:
 * - "scaleFactor" (optional, double): The scale factor for the scene. Defaults to 1.0 if not provided.
 * - "items" (array): A list of graphical items to be added to the scene. Each item is an object containing:
 *   - "type" (string): The type of the item (e.g., "polygon", "ellipse", "line", "light").
 *   - Additional properties specific to the type of item, such as coordinates, size, or color.
 * - "fog" (optional, string): A Base64-encoded PNG image representing the fog of war for the scene.
 *
 * Supported item types:
 * - **polygon**: Represents a filled polygon.
 *   - "points" (array): An array of 2D points (arrays of 2 doubles) defining the polygon vertices.
 *   - "color" (string): The color of the polygon.
 *   - "z" (optional, double): The z-index of the polygon. Defaults to 5.
 * - **ellipse**: Represents a filled ellipse.
 *   - "center" (array): An array of 2 doubles representing the center of the ellipse.
 *   - "radius" (double): The radius of the ellipse.
 *   - "color" (string): The color of the ellipse.
 * - **line**: Represents a line.
 *   - "start" (array): An array of 2 doubles representing the start point of the line.
 *   - "end" (array): An array of 2 doubles representing the end point of the line.
 *   - "color" (string): The color of the line.
 * - **light**: Represents a custom light source.
 *   - "center" (array): An array of 2 doubles representing the center of the light.
 *   - "color" (string): The color of the light.
 *   - "r1" (double): The inner radius of the light source.
 *   - "r2" (double): The outer radius of the light source.
 *
 * If the "fog" key is present in the JSON object, the fog image will be decoded from the Base64 string
 * and applied. The fog of war display will then be updated, with a default opacity of 0.5 and a z-index of 100.
 * Any existing fog of war is removed and replaced with the new one.
 *
 * @param obj The QJsonObject containing the scene data to be parsed and applied.
 */
void MapScene::fromJson(const QJsonObject& obj) {
    m_scaleFactor = obj["scaleFactor"].toDouble(1.0);
    m_gridSize = obj["gridCellSie"].toDouble(5.0);


    QJsonArray itemsArray = obj["items"].toArray();
    int currentProgress, step;
    if (itemsArray.count() > 0)
        step = std::floor((99 - 35) / itemsArray.count());
    currentProgress = 36;
    for (const auto& val : itemsArray) {
        emit progressChanged(currentProgress+=step, tr("Populating map with graphics"));
        QJsonObject itemObj = val.toObject();
        QString type = itemObj["type"].toString();

        if (type == "polygon") {
            QPolygonF polygon;
            for (const auto& pt : itemObj["points"].toArray())
                polygon << QPointF(pt.toArray()[0].toDouble(), pt.toArray()[1].toDouble());

            auto* item = new QGraphicsPolygonItem(polygon);
            item->setPen(QPen(QColor(itemObj["color"].toString())));
            item->setBrush(QBrush(QColor(itemObj["color"].toString()), Qt::Dense4Pattern));
            item->setZValue(itemObj["z"].toDouble(mapLayers::Shapes));
            addItem(item);
        }
        else if (type == "ellipse") {
            QPointF center(itemObj["center"].toArray()[0].toDouble(),
                           itemObj["center"].toArray()[1].toDouble());
            double r = itemObj["radius"].toDouble();
            auto* item = new QGraphicsEllipseItem(QRectF(center.x() - r, center.y() - r, 2 * r, 2 * r));
            QColor color(itemObj["color"].toString());
            item->setPen(QPen(color));
            item->setBrush(QBrush(color, Qt::Dense4Pattern));
            item->setZValue(itemObj["z"].toDouble(mapLayers::Shapes));
            addItem(item);
        }
        else if (type == "line") {
            QLineF line(
                    QPointF(itemObj["start"].toArray()[0].toDouble(), itemObj["start"].toArray()[1].toDouble()),
                    QPointF(itemObj["end"].toArray()[0].toDouble(), itemObj["end"].toArray()[1].toDouble()));
            auto* item = new QGraphicsLineItem(line);
            item->setPen(QPen(QColor(itemObj["color"].toString())));
            item->setZValue(itemObj["z"].toDouble(mapLayers::Shapes));
            addItem(item);
        }
        else if (type == "light") {
            QPointF center(itemObj["center"].toArray()[0].toDouble(),
                           itemObj["center"].toArray()[1].toDouble());
            QColor color(itemObj["color"].toString());
            auto* light = new LightSourceItem(itemObj["r1"].toDouble(), itemObj["r2"].toDouble(), color, center, nullptr);
            addItem(light);
        }
        else if (type == "heightRegion"){
            QPolygonF polygon;
            for (const auto& pt : itemObj["points"].toArray())
                polygon << QPointF(pt.toArray()[0].toDouble(), pt.toArray()[1].toDouble());
            qreal height = itemObj["height"].toDouble(0);
            auto* heightRegion = new HeightRegionItem(polygon, height);
            addItem(heightRegion);
        }
    }

    if (obj.contains("fog")) {
        QByteArray byteArray = QByteArray::fromBase64(obj["fog"].toString().toLatin1());
        QImage img;
        if (img.loadFromData(byteArray, "PNG")) {
            fogImage = img;
        }
    }

    if (obj.contains("gridCellSie"))
        setGridSize(obj["gridCellSie"].toInt());

    if (obj.contains("grid"))
        setGridType(obj["grid"].toInt());

    update();
}

/**
 * @brief Saves the current state of the map scene to a file at the specified path.
 *
 * This function serializes the scene's data (including both map and image information) into a custom
 * binary format and writes it to the provided file path. The saved file includes a header with metadata,
 * followed by JSON data representing the map state, and finally the image data of the map.
 *
 * @param path The file path where the scene data should be saved.
 * @return Returns true if the file was successfully saved; otherwise, returns false.
 *
 * The file-saving process involves:
 * - Creating a file at the provided path.
 * - Serializing the current map state into a JSON object using toJson().
 * - Retrieving the map's visual representation as a pixmap using getMapPixmap() and converting it to PNG format.
 * - Preparing a custom file header (MapFileHeader) that records the sizes of the JSON and image data.
 * - Writing the header, JSON data, and image data to the file using QDataStream.
 *
 * If the file cannot be opened in write mode, the function returns false.
 */
bool MapScene::saveToFile(const QString& path) {
    QFile file(path);
    emit progressChanged(5, tr("Opening file"));
    if (!file.open(QIODevice::WriteOnly)) {
        emit progressChanged(0, tr("Failed to open file"));
        return false;
    }
    emit progressChanged(10, tr("Collecting graphic objects"));
    QJsonObject mapJson = this->toJson();
    QJsonDocument doc(mapJson);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    QPixmap pixmap = getMapPixmap();
    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    pixmap.toImage().save(&buffer, "PNG");

    MapFileHeader header;
    header.jsonSize = jsonData.size();
    header.imageSize = imageData.size();

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    emit progressChanged(80, tr("Writing signature"));
    stream.writeRawData(reinterpret_cast<char*>(&header), sizeof(header));
    emit progressChanged(85, tr("Writing graphics data"));
    stream.writeRawData(jsonData.constData(), jsonData.size());
    emit progressChanged(90, tr("Writing map image"));
    stream.writeRawData(imageData.constData(), imageData.size());

    file.close();
    emit progressChanged(100, tr("Done!"));
    return true;
}

/**
 * @brief Loads and initializes the map information from a file.
 *
 * This function reads a map file specified by the input path and processes its contents,
 * which include metadata, JSON-encoded scene data, and an embedded map image. The function
 * performs several validation checks during the file reading process, such as verifying
 * the file signature and size integrity of read data.
 *
 * @param path The path to the map file to be loaded.
 * @return Returns an integer status code indicating the result of the operation, which may be:
 * - qmapErrorCodes::NoError (0): If the file is successfully loaded and processed.
 * - qmapErrorCodes::FileOpenError: If there is an error opening the file or insufficient data is read.
 * - qmapErrorCodes::FileSignatureError: If the file does not contain the expected map signature.
 * - qmapErrorCodes::JsonParseError: If the JSON block cannot be parsed successfully.
 *
 * The function performs the following steps:
 * 1. Attempts to open the file for reading. If unsuccessful, returns qmapErrorCodes::FileOpenError.
 * 2. Reads the file header and verifies its signature to ensure it matches the expected magic number.
 * 3. Extracts the JSON data and image data of the specified sizes from the file. If data sizes
 *    are incorrect, returns qmapErrorCodes::FileOpenError.
 * 4. Parses the JSON data using QJsonDocument. If JSON is invalid, returns qmapErrorCodes::JsonParseError.
 * 5. Loads the extracted image data into a QImage object. If successful, the scene is cleared,
 *    and the new map image is added as a QGraphicsPixmapItem with appropriate layer settings.
 * 6. Calls the fromJson function to initialize scene items from the JSON object.
 * 7. Initializes the fog of war based on the dimensions of the map image, if applicable.
 * 8. Closes the file and returns qmapErrorCodes::NoError upon successful completion.
 */
int MapScene::loadFromFile(const QString& path) {
    emit progressChanged(0, tr("Opening file"));
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Не удалось открыть файл для чтения:" << path;
        return mapErrorCodes::FileOpenError;
    }
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);


    MapFileHeader header;
    if (stream.readRawData(reinterpret_cast<char*>(&header), sizeof(header)) != sizeof(header))
        return mapErrorCodes::FileOpenError;

    emit progressChanged(5, tr("Checking signature"));
    if (header.magic != 0x444D414D) {
        qWarning() << "Файл не является картой DM-Assist.";
        return mapErrorCodes::FileSignatureError;
    }

    QByteArray jsonData(header.jsonSize, 0);
    QByteArray imageData(header.imageSize, 0);

    if (stream.readRawData(jsonData.data(), header.jsonSize) != header.jsonSize ||
        stream.readRawData(imageData.data(), header.imageSize) != header.imageSize) {
        return false;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Ошибка разбора JSON:" << error.errorString();
        return mapErrorCodes::JsonParseError;
    }

    emit progressChanged(30, tr("Loading map image"));
    QImage mapImage;
    mapImage.loadFromData(imageData, "PNG");

    if (mapImage.isNull())
        return mapErrorCodes::NoDataError;

    clear();
    QGraphicsPixmapItem* pixmapItem = addPixmap(QPixmap::fromImage(mapImage));
    pixmapItem->setZValue(mapLayers::Background);

    emit progressChanged(35, tr("Initializing fog"));
    initializeFog(mapImage.size());
    m_lineWidth = pixmapItem->boundingRect().height() / 200;

    fromJson(doc.object());
    file.close();
    emit progressChanged(99, tr("Initializing grid"));
    initializeGrid();
    emit progressChanged(100, tr("Done!"));
    return mapErrorCodes::NoError;
}

/**
 * @brief Returns the rectangular bounds of the map as a QRectF object.
 *
 * This method retrieves the pixmap of the first QGraphicsPixmapItem in the scene
 * (excluding the fog item) by calling getMapPixmap() and fetches its rectangular bounds.
 * If no QPixmap is found, the returned rectangle will correspond to the default bounds
 * of an empty QPixmap.
 *
 * @return A QRectF representing the bounds of the map pixmap.
 */
QRectF MapScene::mapRect() const {
    QPixmap pixmap = getMapPixmap();
    return pixmap.rect();
}

qreal MapScene::heightAt(const QPointF &pos) const {
    for (QGraphicsItem* item : items(pos)) {
        auto *region = dynamic_cast<HeightRegionItem*>(item);
        if (region && region->contains(region->mapFromScene(pos)))
            return region->height();
    }
    return 0.0;
}


void MapScene::enableGrid(bool enabled) {
    m_gridEnabled = enabled;
    if (m_gridItem)
        m_gridItem->setVisible(m_gridEnabled);
}

void MapScene::setGridType(int gridType) {
    m_gridType = gridType;
    enableGrid(gridType != GridItem::GridType::None);
    if (m_gridItem && m_gridEnabled)
        m_gridItem->setGridType(gridType);
}

void MapScene::setGridSize(int feet) {
    m_gridSize = qMax<qreal>(feet, 0.01);
    if (m_gridItem)
        m_gridItem->setCellFeet(m_gridSize);

    for (auto* item : items()) {
        if (item->zValue() == mapLayers::Tokens){
            if (auto* token = dynamic_cast<TokenItem*>(item))
                token->setGridStep(feet);
        }
    }
}

void MapScene::initializeGrid() {
    if (!m_gridItem){
        m_gridItem = new GridItem(mapRect());
        m_gridItem->setZValue(mapLayers::Grid);
        addItem(m_gridItem);
        m_gridItem->setPixelsPerFoot(1/m_scaleFactor);
        m_gridItem->setCellFeet(m_gridSize);
        m_gridItem->setGridType(m_gridType);
        m_gridItem->setVisible(false);
    }
}

void MapScene::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
{
    if (event->mimeData()->hasFormat("application/x-character"))
        event->acceptProposedAction();
}

void MapScene::dropEvent(QGraphicsSceneDragDropEvent* event)
{
    if (!event->mimeData()->hasFormat("application/x-character"))
        return;

    QByteArray data = event->mimeData()->data("application/x-character");
    QString jsonPath = QString::fromUtf8(data);

    QFile f(jsonPath);
    if (!f.open(QIODevice::ReadOnly)) return;

    TokenStruct tokenStruct = TokenItem::fromJson(jsonPath);
    addToken(tokenStruct, jsonPath, event->scenePos());

    event->acceptProposedAction();
}

void MapScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event) {
    bool inside = sceneRect().contains(event->scenePos());
    if (event->mimeData()->hasFormat("application/x-character") && inside)
        event->acceptProposedAction();
    else
        event->ignore();
}

void MapScene::addToken(const TokenStruct &tokenStruct, const QString &filePath, QPointF pos) {
    QPixmap tokenPixmap;
    if (!tokenStruct.imgPath.isEmpty() && QFile::exists(tokenStruct.imgPath))
        tokenPixmap.load(tokenStruct.imgPath);
    else
        tokenPixmap.load(":/map/default-token.png");

    auto* token = new TokenItem(filePath, tokenStruct.name, tokenPixmap, tokenStruct.size, 1 / m_scaleFactor);

    connect(token, &TokenItem::openCharSheet, this, &MapScene::openCharseetRequested);
    connect(token, &TokenItem::addToTracker, this, &MapScene::addToEncounterRequested);
    token->setTitleDisplayMode(m_tokenMode);

    token->setZValue(mapLayers::Tokens);
    addItem(token);
    token->setPos(pos);
}

void MapScene::setTokenTitleMode(int mode) {
    m_tokenMode = mode;

    for (auto* pItem : items()) {
        if (auto* token = dynamic_cast<TokenItem*>(pItem))
            token->setTitleDisplayMode(m_tokenMode);
    }
}

void MapScene::setTokenTextSize(int size) {
    for (auto* pItem : items()) {
        if (auto* token = dynamic_cast<TokenItem*>(pItem))
            token->setFontSize(size);
    }
}

QPointF MapScene::snapToGrid(const QPointF &pos, qreal objSizeFeet) const {
    if (!m_gridItem || !m_gridEnabled) return pos;

    const int nCells = qMax(1, int(std::llround(objSizeFeet / m_gridSize)));
    QRectF gridRect = m_gridItem->boundingRect();
    QPointF origin = gridRect.topLeft();

    if (m_gridType == GridItem::GridType::Square){
        const qreal step = m_gridSize / m_scaleFactor;  ///< px per cell
        const qreal halfSizePx = (nCells * step) / 2.0;

        qreal topLeftX = std::floor((pos.x() - halfSizePx - origin.x()) / step) * step + origin.x();
        qreal topLeftY = std::floor((pos.y() - halfSizePx - origin.y()) / step) * step + origin.y();
        qreal centerX = topLeftX + (nCells * step) / 2.0;
        qreal centerY = topLeftY + (nCells * step) / 2.0;
        return QPointF(centerX, centerY);
    } else {
        const qreal  flatToFlatPx = m_gridSize / m_scaleFactor;
        const qreal s = flatToFlatPx / std::sqrt(3.0);
        const qreal px = pos.x() - origin.x();
        const qreal py = pos.y() - origin.y();

        qreal qf = ((std::sqrt(3.0) / 3.0) * px - (1.0/3.0) * py) / s;
        qreal rf = ((2.0/3.0) * py) / s;


        qreal x = qf;
        qreal z = rf;
        qreal y = -x -z;

        long rx = std::llround(x);
        long ry = std::llround(y);
        long rz = std::llround(z);

        double dx = std::fabs(rx - x);
        double dy = std::fabs(ry - y);
        double dz = std::fabs(rz - z);

        if (dx > dy && dx > dz) rx = -ry -rz;
        else if (dy > dz && dy > dx) ry = -rx - rz;
        else rz = -rx - ry;

        long q = rx;
        long r = rz;

        double centerX = s * std::sqrt(3.0) * (q + r / 2.0) + origin.x();
        double centerY = s * 1.5 * r + origin.y();

        return QPointF(centerX, centerY);
    }
}
