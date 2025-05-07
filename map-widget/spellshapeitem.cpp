#include "spellshapeitem.h"
#include <QPainter>
#include <QtMath>

SpellShapeItem::SpellShapeItem(SpellShapeType type, QPointF p1, QPointF p2, QColor color)
        : shapeType(type), point1(p1), point2(p2), fillColor(color) {
    setZValue(15);
}

QRectF SpellShapeItem::boundingRect() const {
    return QRectF(point1, point2).normalized().adjusted(-10, -10, 10, 10); // чуть больше для хвоста пера
}

void SpellShapeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setRenderHint(QPainter::Antialiasing);
    fillColor.setAlpha(180);
    QBrush brush(fillColor, Qt::SolidPattern);
    brush.setColor(fillColor.lighter(160));
    painter->setBrush(brush);
    painter->setPen(QPen(fillColor, 2));

    switch (shapeType) {
        case SpellShapeType::Line:
            painter->drawLine(point1, point2);
            break;
        case SpellShapeType::Circle: {
            qreal radius = QLineF(point1, point2).length();
            painter->drawEllipse(point1, radius, radius);
            break;
        }
        case SpellShapeType::Square: {
            QPointF center = point1;
            QPointF diag = point2 - point1;
            QPointF offset = QPointF(diag.x(), diag.y());
            painter->drawRect(QRectF(center - offset, center + offset));
            break;
        }
        case SpellShapeType::Triangle: {
            QLineF base(point1, point2);
            QPointF mid = (point1 + point2) / 2;
            QLineF perp(base.p2(), base.p1());
            perp.setLength(base.length() * qTan(M_PI / 3)); // 60° угол
            perp.setAngle(base.angle() - 90);
            QPointF third = mid + perp.p2() - perp.p1();
            QPolygonF triangle;
            triangle << point1 << point2 << third;
            painter->drawPolygon(triangle);
            break;
        }
    }
}

void SpellShapeItem::updatePoints(QPointF p1, QPointF p2) {
    prepareGeometryChange();
    point1 = p1;
    point2 = p2;
    update();
}
