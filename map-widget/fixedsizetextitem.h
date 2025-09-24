#ifndef DM_ASSIST_FIXEDSIZETEXTITEM_H
#define DM_ASSIST_FIXEDSIZETEXTITEM_H

#include <QGraphicsTextItem>
#include <QPainter>

class FixedSizeTextItem : public QGraphicsTextItem {
public:
    using QGraphicsTextItem::QGraphicsTextItem;

    void setOutlineColor(const QColor& color) { outlineColor = color; }
    void setOutlineWidth(qreal w) { outlineWidth = w; }

    void paint(QPainter* p, const QStyleOptionGraphicsItem* opt, QWidget* w) override;


private:
    QColor outlineColor = Qt::black;
    qreal outlineWidth = 2.0;
};


#endif //DM_ASSIST_FIXEDSIZETEXTITEM_H
