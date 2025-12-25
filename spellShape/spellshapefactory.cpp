#include "spellshapefactory.h"

#include "effectgrapchicsitem.h"
#include <QGraphicsLineItem>
#include <../map-widget/mapscene.h>

QGraphicsItem *SpellShapeFactory::createShape(QGraphicsScene *scene, const SpellShapeFactory::Params &p) {
    const qreal r = p.feetSize * p.feetToPx;

    QGraphicsItem* item = nullptr;

    switch (p.type) {
        case SpellShapeType::Circle: {
            QRectF rect(-r, -r, r*2, r*2);
            item = new EffectEllipseItem(rect, p.color, p.textureName);
            break;
        }
        case SpellShapeType::Square: {
            QRectF rect(-r, -r, r*2, r*2);
            item = new EffectPolygonItem(QPolygonF(rect), p.color, p.textureName);
            break;
        }
        case SpellShapeType::Line: {
            auto* line = new QGraphicsLineItem(QLineF(0, 0, r, 0));
            line->setPen(QPen(p.color, 0));
            item = line;
            break;
        }
        case SpellShapeType::Triangle: {
            QPolygonF poly;
            poly << QPointF(0, 0) << QPointF(r, -r/2) << QPointF(r, r/2);
            item = new EffectPolygonItem(poly, p.color, p.textureName);
            break;
        }
    }

    if (!item) return nullptr;

    item->setOpacity(p.opacity);
    item->setZValue(mapLayers::Shapes);
    scene->addItem(item);
    return item;
}

QRectF SpellShapeFactory::shapeBoudingRect(SpellShapeType type, qreal feetSize, qreal feetToPx) {
    const qreal r = feetSize * feetToPx;
    switch (type) {
        case SpellShapeType::Circle:
        case SpellShapeType::Square:
            return QRectF(-r, -r, r*2, r*2);
        case SpellShapeType::Line:
            return QRectF(0, -r*0.1, r, r*0.2);
        case SpellShapeType::Triangle:
            return QRectF(0, -r/2, r, r);
    }
    return {};
}


