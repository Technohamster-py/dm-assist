#ifndef DM_ASSIST_SPELLSHAPEITEM_H
#define DM_ASSIST_SPELLSHAPEITEM_H


#include <QGraphicsItem>
#include <QColor>

enum class SpellShapeType {
    Line,
    Circle,
    Square,
    Triangle
};

class SpellShapeItem : public QGraphicsItem {
public:
    SpellShapeItem(SpellShapeType type, QPointF p1, QPointF p2, QColor color);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

    void updatePoints(QPointF p1, QPointF p2);
private:
    SpellShapeType shapeType;
    QPointF point1;
    QPointF point2;
    QColor fillColor;
};


#endif //DM_ASSIST_SPELLSHAPEITEM_H
