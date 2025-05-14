/**
 * @file mapscene.cpp
 * @brief Implementation of MapScene class.
 */

#include "mapscene.h"
#include "lighttool.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QPainter>
#include <QKeyEvent>
#include <QJsonArray>
#include <QBuffer>
#include "lighttool.h"

/**
 * @brief Constructs a new MapScene.
 * @param parent Optional parent QObject
 */
MapScene::MapScene(QObject *parent)
        : QGraphicsScene(parent)
{
    setBackgroundBrush(Qt::darkGray);
}

void MapScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (m_activeTool) {
        m_activeTool->mousePressEvent(event, this);
    } else {
        QGraphicsScene::mousePressEvent(event);
    }
}

void MapScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (m_activeTool) {
        m_activeTool->mouseMoveEvent(event, this);
    } else {
        QGraphicsScene::mouseMoveEvent(event);
    }
}


void MapScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (m_activeTool){
        m_activeTool->mouseReleaseEvent(event, this);
    } else {
        QGraphicsScene::mouseReleaseEvent(event);
    }

}

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
}

void MapScene::setActiveTool(AbstractMapTool *tool) {
    if (m_activeTool)
        m_activeTool->deactivate(this);
    m_activeTool = tool;
    emit toolChanged(tool);
}

void MapScene::initializeFog(const QSize &size) {
    fogImage = QImage(size, QImage::Format_ARGB32_Premultiplied);
    fogImage.fill(Qt::transparent); // всё видно по умолчанию

    if (fogItem) {
        removeItem(fogItem);
        delete fogItem;
    }

    fogItem = addPixmap(QPixmap::fromImage(fogImage));
    fogItem->setZValue(10); // поверх карты
    fogItem->setOpacity(0.5); // прозрачность по умолчанию (для мастера)
}

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

    if (fogItem) {
        fogItem->setPixmap(QPixmap::fromImage(fogImage));
    }
    emit fogUpdated(fogImage);
}

void MapScene::setFogOpacity(qreal opacity) {
    if (fogItem) {
        fogItem->setOpacity(opacity);
    } else {
        qWarning("Fog item not initialized!");
    }
}

QPixmap MapScene::getMapPixmap() const {
    auto mapItems = items(Qt::AscendingOrder);
    for (auto *item : mapItems) {
        if (auto pixmapItem = qgraphicsitem_cast<QGraphicsPixmapItem *>(item)) {
            if (pixmapItem != fogItem) {
                return pixmapItem->pixmap();
            }
        }
    }
    return QPixmap();
}

void MapScene::drawFogPath(const QPainterPath &path, bool hide) {
    QPainter painter(&fogImage);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(hide ? Qt::black : Qt::transparent);
    painter.setCompositionMode(hide ? QPainter::CompositionMode_SourceOver
                                    : QPainter::CompositionMode_Clear);
    painter.drawPath(path);
    painter.end();

    updateFog();
}

void MapScene::clearFog() {
    fogImage.fill(Qt::transparent);

    updateFog();
}

void MapScene::updateFog() {
    if (fogItem) {
        fogItem->setPixmap(QPixmap::fromImage(fogImage));
    }
    emit fogUpdated(fogImage);
}

void MapScene::drawScaledCircle(const QPointF &scenePos, int radius, bool hide) {
    int realRadius = static_cast<int>(radius / m_scaleFactor);
    drawFogCircle(scenePos, realRadius, hide);
}

void MapScene::keyPressEvent(QKeyEvent *event) {
    if (event->matches(QKeySequence::Undo)) {
        undoLastAction();  // реализация ниже
        return;
    }
    QGraphicsScene::keyPressEvent(event);
}

void MapScene::addUndoableItem(QGraphicsItem *item) {
    addItem(item);
    undoStack.push(std::make_unique<AddItemAction>(item));
}

void MapScene::removeUndoableItem(QGraphicsItem *item) {
    undoStack.push(std::make_unique<RemoveItemAction>(item));
    removeItem(item);
}

void MapScene::undoLastAction() {
    undoStack.undo(this);
}

QJsonObject MapScene::toJson() const {
    QJsonObject rootObject;
    rootObject["scaleFactor"] = m_scaleFactor;


    QJsonArray itemsArray;
    for (auto item : items()) {
        QJsonObject itemObj;

        if (auto* polyItem = qgraphicsitem_cast<QGraphicsPolygonItem*>(item)) {
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
        }
        else if (auto* line = qgraphicsitem_cast<QGraphicsLineItem*>(item)) {
            itemObj["type"] = "ruler";
            QLineF l = line->line();
            itemObj["start"] = QJsonArray{ l.p1().x(), l.p1().y() };
            itemObj["end"]   = QJsonArray{ l.p2().x(), l.p2().y() };
            itemObj["color"] = line->pen().color().name();
        }
        else if (auto* light = dynamic_cast<LightSourceItem*>(item)) {
            itemObj["type"] = "light";
            itemObj["position"] = QJsonArray{ light->pos().x(), light->pos().y() };
            itemObj["r1"] = light->brightRadius();
            itemObj["r2"] = light->dimRadius();
            itemObj["color"] = light->color().name();
        }

        if (!itemObj.isEmpty())
            itemsArray.append(itemObj);
    }

    rootObject["items"] = itemsArray;

    if (!fogImage.isNull()) {
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        buffer.open(QIODevice::WriteOnly);
        fogImage.save(&buffer, "PNG");
        rootObject["fog"] = QString::fromLatin1(byteArray.toBase64());
    }

    return rootObject;
}

void MapScene::fromJson(const QJsonObject& obj) {
    clear();
    m_scaleFactor = obj["scaleFactor"].toDouble(1.0);


    QJsonArray itemsArray = obj["items"].toArray();
    for (const auto& val : itemsArray) {
        QJsonObject itemObj = val.toObject();
        QString type = itemObj["type"].toString();

        if (type == "polygon") {
            QPolygonF polygon;
            for (const auto& pt : itemObj["points"].toArray())
                polygon << QPointF(pt.toArray()[0].toDouble(), pt.toArray()[1].toDouble());

            auto* item = new QGraphicsPolygonItem(polygon);
            item->setPen(QPen(QColor(itemObj["color"].toString())));
            item->setBrush(QBrush(QColor(itemObj["color"].toString()), Qt::Dense4Pattern));
            item->setZValue(itemObj["z"].toDouble(5));
            addItem(item);
        }
        else if (type == "ellipse") {
            QPointF center(itemObj["center"].toArray()[0].toDouble(),
                           itemObj["center"].toArray()[1].toDouble());
            double r = itemObj["radius"].toDouble();
            auto* item = new QGraphicsEllipseItem(QRectF(center.x() - r, center.y() - r, 2 * r, 2 * r));
            item->setPen(QPen(QColor(itemObj["color"].toString())));
            item->setBrush(QBrush(Qt::transparent));
            addItem(item);
        }
        else if (type == "ruler") {
            QLineF line(
                    QPointF(itemObj["start"].toArray()[0].toDouble(), itemObj["start"].toArray()[1].toDouble()),
                    QPointF(itemObj["end"].toArray()[0].toDouble(), itemObj["end"].toArray()[1].toDouble()));
            auto* item = new QGraphicsLineItem(line);
            item->setPen(QPen(QColor(itemObj["color"].toString())));
            addItem(item);
        }
        else if (type == "light") {
            QPointF center(itemObj["center"].toArray()[0].toDouble(),
                           itemObj["center"].toArray()[1].toDouble());
            QColor color(itemObj["color"].toString());
            auto* light = new LightSourceItem(itemObj["r1"].toDouble(), itemObj["r2"].toDouble(), color, center, nullptr);
            addItem(light);
        }
    }

    if (obj.contains("fog")) {
        QByteArray byteArray = QByteArray::fromBase64(obj["fog"].toString().toLatin1());
        QImage img;
        if (img.loadFromData(byteArray, "PNG")) {
            fogImage = img;
            updateFog(); // ваша функция для отображения тумана на сцене
        }
    }
}


bool MapScene::saveToQmap(const QString& filePath, const QImage& mapImage) {
    QuaZip zip(filePath);
    if (!zip.open(QuaZip::mdCreate)) return false;

    // JSON файл
    QuaZipFile jsonFile(&zip);
    if (!jsonFile.open(QIODevice::WriteOnly, QuaZipNewInfo("map.json")))
        return false;

    QJsonDocument doc(toJson());
    jsonFile.write(doc.toJson(QJsonDocument::Compact));
    jsonFile.close();

    // Фоновое изображение
    QuaZipFile imageFile(&zip);
    if (!imageFile.open(QIODevice::WriteOnly, QuaZipNewInfo("map.png")))
        return false;

    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    mapImage.save(&buffer, "PNG");

    imageFile.write(imageData);
    imageFile.close();
    zip.close();

    return true;
}
