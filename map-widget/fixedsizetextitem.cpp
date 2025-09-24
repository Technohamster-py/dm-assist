#include "fixedsizetextitem.h"

void FixedSizeTextItem::paint(QPainter *p, const QStyleOptionGraphicsItem *opt, QWidget *w) {
    QTransform m = p->worldTransform();
    qreal sx = std::sqrt(m.m11()*m.m11() + m.m21()*m.m21());
    qreal sy = std::sqrt(m.m22()*m.m22() + m.m12()*m.m12());

    p->save();
    if (!qFuzzyIsNull(sx) && !qFuzzyIsNull(sy))
        p->scale(1.0/sx, 1.0/sy);

    //Center positioning
    QFontMetrics fm(font());
    QString text = toPlainText();
    qreal textWidth = fm.horizontalAdvance(text);
    qreal textHeight = fm.height();
    qreal x = -textWidth / 2.0;
    qreal y = textHeight;

    // Outline
    QPainterPath path;
    path.addText(x, y, font(), text);
    p->setPen(QPen(outlineColor, outlineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p->setBrush(Qt::NoBrush);
    p->drawPath(path);


    // Main text
    p->setPen(Qt::NoPen);
    p->setBrush(defaultTextColor());
    p->drawPath(path);

    p->restore();
}

QRectF FixedSizeTextItem::boundingRect() const {
    QFontMetrics fm(font());
    QString text = toPlainText();
    qreal w = fm.horizontalAdvance(text);
    qreal h = fm.height();

    return QRectF(-w / 2.0 - 4, -h, w + 8, h + 8);
}
