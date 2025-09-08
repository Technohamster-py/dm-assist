#ifndef DM_ASSIST_TOKENITEM_H
#define DM_ASSIST_TOKENITEM_H

#include <QGraphicsObject>

class TokenItem : public QGraphicsObject
{
    Q_OBJECT
public:
    TokenItem(const QString& name, const QPixmap& pixmap, qreal realSize, qreal pxPerFoot);

    QRectF boundingRect() const override { return childrenBoundingRect(); }
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override {}

signals:
    void addToTracker(TokenItem*);
    void openCharSheet(TokenItem*);

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QGraphicsPixmapItem* pixmapItem;
    QGraphicsSimpleTextItem* labelItem;
    qreal m_realSize;
    qreal m_pxPerFoot;
};



#endif //DM_ASSIST_TOKENITEM_H
