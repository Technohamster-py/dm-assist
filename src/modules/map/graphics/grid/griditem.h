#ifndef DM_ASSIST_GRIDITEM_H
#define DM_ASSIST_GRIDITEM_H

#include <QGraphicsItem>
#include <QPen>
#include <QtMath>

class GridItem : public QGraphicsItem
{
public:
    enum GridType {None=0, Square, Hex };

    explicit GridItem(const QRectF& size, QGraphicsItem *parent = nullptr);

    void setGridType(int t)            { m_type = t; update(); }
    void setCellFeet(qreal feet)            { m_cellFeet = feet; update(); }
    void setPixelsPerFoot(qreal ppf)        { m_pixelsPerFoot = qMax<qreal>(ppf, 0.0001); update(); }
    void setLineColor(const QColor& c)      { m_pen.setColor(c); update(); }

    QRectF boundingRect() const override { return m_rect; }
    void paint(QPainter *p, const QStyleOptionGraphicsItem *opt, QWidget *w) override;

    static int modesCount() {return GridType::Hex;}
    static QString stringMode(int mode);
private:
    static void paintSquareGrid(QPainter* p, const QRectF& rect, qreal stepPx);
    static void paintHexGrid(QPainter* p, const QRectF& rect, qreal flatToFlatPx);

private:
    QRectF m_rect;
    int  m_type = GridType::Square;
    qreal m_cellFeet = 5.0;
    qreal m_pixelsPerFoot = 1.0;
    QPen  m_pen = QPen(QColor(200,200,200,160), 0.0); // hairline
};


#endif //DM_ASSIST_GRIDITEM_H
