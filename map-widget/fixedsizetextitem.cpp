#include "fixedsizetextitem.h"

void FixedSizeTextItem::paint(QPainter *p, const QStyleOptionGraphicsItem *opt, QWidget *w) {
    QTransform m = p->worldTransform();
    qreal sx = std::sqrt(m.m11()*m.m11() + m.m21()*m.m21());
    qreal sy = std::sqrt(m.m22()*m.m22() + m.m12()*m.m12());

    p->save();
    if (!qFuzzyIsNull(sx) && !qFuzzyIsNull(sy))
        p->scale(1.0/sx, 1.0/sy);

    QPainterPath path;
    QFontMetrics fm(font());
    path.addText(0, fm.ascent(), font(), toPlainText());
    p->setPen(QPen(outlineColor, outlineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p->setBrush(Qt::NoBrush);
    p->drawPath(path);

    p->setPen(Qt::NoPen);
    p->setBrush(defaultTextColor());
    p->drawPath(path);

    p->restore();
}
