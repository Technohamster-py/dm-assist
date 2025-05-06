#include "lighttool.h"
#include "mapscene.h"
#include <QPainter>
#include <QRadialGradient>
#include <QStyleOptionProgressBar>

#include <QDebug>

LightSourceItem::LightSourceItem(qreal r1, qreal r2, QColor color, QPointF pos, LightTool *tool)
        : radiusBright(r1), radiusDim(r2), lightColor(color), center(pos), m_tool(tool){
    setPos(center);
    setZValue(11);

    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
    setFlag(QGraphicsItem::ItemIsFocusable);
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

void LightSourceItem::handleMousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        scene()->removeItem(this);
        auto mapScene = dynamic_cast<MapScene*>(scene());
        if (mapScene && m_tool && m_tool->autoUpdateFog()) {
            mapScene->drawFogCircle(lastScenePos, radiusDim + 2, true);
        }
        delete this;
        return;
    }

    if (event->button() == Qt::LeftButton && (event->modifiers() & Qt::ControlModifier)) {
        dragging = true;
        lastScenePos = scenePos();
        dragStart = event->scenePos();
        event->accept();
        return;
    } else {
        QGraphicsItem::mousePressEvent(event);
    }
}

void LightSourceItem::handleMouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (dragging) {
        QPointF delta = event->scenePos() - dragStart;
        setPos(pos() + delta);
        dragStart = event->scenePos();
        event->accept();
        return;
    }
}

void LightSourceItem::handleMouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (dragging) {
        dragging = false;
        auto mapScene = dynamic_cast<MapScene*>(scene());
        if (mapScene && m_tool && m_tool->autoUpdateFog()) {
            mapScene->drawFogCircle(lastScenePos, radiusDim + 2, true);
            mapScene->drawFogCircle(scenePos(), radiusDim, false);
        } else if (mapScene) {
            mapScene->drawFogCircle(scenePos(), radiusDim, false);
        }
    } else {
        QGraphicsItem::mouseReleaseEvent(event);
    }
}



LightTool::LightTool(QObject *parent) : AbstractMapTool(parent) {}

void LightTool::setBrightRadius(int r1) { m_brightRadius = r1; }
void LightTool::setDimRadius(int r2) { m_dimRadius = r2; }
void LightTool::setColor(QColor c) { m_color = c; }

void LightTool::mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    LightSourceItem* lightSourceItem = dynamic_cast<LightSourceItem*>(scene->itemAt(event->scenePos(), QTransform()));
    if (lightSourceItem) {
        lightSourceItem->handleMousePressEvent(event);
        return;
    }

    auto mapScene = dynamic_cast<MapScene*>(scene);
    if (!mapScene) return;

    QPointF pos = event->scenePos();

    auto item = new LightSourceItem(m_brightRadius, m_dimRadius, m_color, pos, this);
    scene->addItem(item);

    mapScene->drawScaledCircle(event->scenePos(), m_dimRadius, false);
}

void LightTool::setAutoUpdateFog(bool enabled) {
    m_autoUpdateFog = enabled;
}

void LightTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    LightSourceItem* lightSourceItem = dynamic_cast<LightSourceItem*>(scene->itemAt(event->scenePos(), QTransform()));
    if (lightSourceItem) {
        lightSourceItem->handleMouseMoveEvent(event);
        return;
    }
}

void LightTool::mouseReleaseEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    LightSourceItem* lightSourceItem = dynamic_cast<LightSourceItem*>(scene->itemAt(event->scenePos(), QTransform()));
    if (lightSourceItem) {
        lightSourceItem->handleMouseReleaseEvent(event);
        return;
    }
}
