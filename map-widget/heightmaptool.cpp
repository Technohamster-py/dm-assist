#include "heightmaptool.h"

#include <QInputDialog>

HeightRegionItem::HeightRegionItem(const QPolygonF &poly, qreal height) : QGraphicsPolygonItem(poly), m_height(height){
    setZValue(mapLayers::Height);
    setPen(Qt::NoPen);
    setOpacity(0.4);
    updateColor();
}

void HeightRegionItem::setHeight(qreal h) {
    m_height = h;
    updateColor();
}

QColor HeightRegionItem::heightToColor(qreal h) {
    qreal  norm = qBound(-100.0, h, 100.0);
    qreal  t = (norm + 100) / 200.0;
    QColor low = QColor::fromRgb(0, 0, 255);
    QColor high = QColor::fromRgb(255, 0, 0);
    return QColor::fromRgbF(low.redF() * (1-t) + high.redF() * t,
                            low.greenF() * (1-t) + high.greenF() * t,
                            low.blueF() * (1-t) + high.blueF() * t);
}

void HeightRegionItem::updateColor() {
    setBrush(heightToColor(m_height));
}



void HeightMapTool::mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene * scene) {
    path = QPainterPath(event->scenePos());
    preview = new QGraphicsPathItem();

    preview->setPen(QPen(Qt::darkGray, 2, Qt::DashLine));
    preview->setZValue(mapLayers::Brush);
    scene->addItem(preview);
}

void HeightMapTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    path.lineTo(event->scenePos());
    if (preview)
        preview->setPath(path);
}

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

    static_cast<MapScene*>(scene)->addUndoableItem(region);
}
