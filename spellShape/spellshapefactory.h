#ifndef DM_ASSIST_SPELLSHAPEFACTORY_H
#define DM_ASSIST_SPELLSHAPEFACTORY_H

#include <QColor>
#include <QGraphicsItem>
#include <QGraphicsScene>

enum class SpellShapeType {
    Line,
    Triangle,
    Circle,
    Square,
};

class SpellShapeFactory {
public:
    struct Params {
        SpellShapeType type;
        int feetSize;
        qreal feetToPx;
        QColor color;
        QString textureName;
        qreal opacity = 0.6;
    };

    static QGraphicsItem* createShape(QGraphicsScene* scene, const Params& p);
    static QRectF shapeBoudingRect(SpellShapeType type, qreal feetSize, qreal feetToPx);
};


#endif //DM_ASSIST_SPELLSHAPEFACTORY_H
