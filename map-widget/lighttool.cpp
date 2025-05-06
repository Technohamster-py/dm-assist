#include "lighttool.h"
#include "mapscene.h"
#include <QPainter>
#include <QRadialGradient>
#include <QStyleOptionProgressBar>


LightSourceItem::LightSourceItem(qreal r1, qreal r2, QColor color, QPointF pos, LightTool *tool)
        : radiusBright(r1), radiusDim(r2), lightColor(color), center(pos), m_tool(tool){
    setPos(center);
    setZValue(11);

    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFlag(QGraphicsItem::ItemIsSelectable); // если нужно
    setFlag(QGraphicsItem::ItemIsMovable);    // даже если у тебя своё перемещение
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
    setFlag(QGraphicsItem::ItemIsFocusable);       // если нужно получать фокус
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

void LightSourceItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        scene()->removeItem(this);
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

void LightSourceItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (dragging) {

        QPointF delta = event->scenePos() - dragStart;
        setPos(pos() + delta);
        dragStart = event->scenePos();
        event->accept();
        return;
    } else {
        QGraphicsItem::mouseMoveEvent(event);
    }
}

void LightSourceItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (dragging) {
        dragging = false;
        auto mapScene = dynamic_cast<MapScene*>(scene());
        if (mapScene && m_tool && m_tool->autoUpdateFog()) {
            mapScene->drawScaledCircle(lastScenePos, radiusDim + 2, true);
            mapScene->drawScaledCircle(scenePos(), radiusDim, false);
        } else if (mapScene) {
            mapScene->drawScaledCircle(scenePos(), radiusDim, false);
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
    QPointF point = event->scenePos();
    LightSourceItem* lightSourceItem = dynamic_cast<LightSourceItem*>(scene->itemAt(point, QTransform()));
    if (lightSourceItem) {
        return;
    }
    auto mapScene = dynamic_cast<MapScene*>(scene);
    if (!mapScene) return;

    QPointF pos = event->scenePos();

    auto item = new LightSourceItem(m_brightRadius, m_dimRadius, m_color, pos, this);
    scene->addItem(item);

    mapScene->drawScaledCircle(pos, m_dimRadius, false);
}

void LightTool::setAutoUpdateFog(bool enabled) {
    m_autoUpdateFog = enabled;
}
