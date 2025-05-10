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
    if (event->key() == Qt::Key_Escape) {
        if (m_activeTool) {
            m_activeTool->cancel();  // каждый инструмент должен реализовать cancel()
            return;
        }
    }

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
