#include "lighttool.h"
#include "mapscene.h"
#include <QPainter>
#include <QRadialGradient>

LightSourceItem::LightSourceItem(qreal r1, qreal r2, QColor color, QPointF pos)
        : radiusBright(r1), radiusDim(r2), lightColor(color), center(pos) {
    setPos(center);
    setZValue(9);
}

QRectF LightSourceItem::boundingRect() const {
    return QRectF(-radiusDim, -radiusDim, 2 * radiusDim, 2 * radiusDim);
}

void LightSourceItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    QRadialGradient gradient(QPointF(0, 0), radiusDim);
    QColor transparent = lightColor;
    transparent.setAlpha(0);

    gradient.setColorAt(0, lightColor);
    gradient.setColorAt(radiusBright / radiusDim, lightColor);
    gradient.setColorAt(1, transparent);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(gradient);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(boundingRect());
}


LightTool::LightTool(QObject *parent) : AbstractMapTool(parent) {}

void LightTool::setBrightRadius(int r1) { m_brightRadius = r1; }
void LightTool::setDimRadius(int r2) { m_dimRadius = r2; }
void LightTool::setColor(QColor c) { m_color = c; }

void LightTool::mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    auto mapScene = dynamic_cast<MapScene*>(scene);
    if (!mapScene) return;

    QPointF pos = event->scenePos();

    auto item = new LightSourceItem(m_brightRadius, m_dimRadius, m_color, pos);
    scene->addItem(item);

    // Удаляем туман в радиусе слабого света
    mapScene->drawFogCircle(pos, m_dimRadius, false);
}