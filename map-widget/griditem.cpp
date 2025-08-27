#include "griditem.h"

GridItem::GridItem(MapScene *scene) : QGraphicsItem(), m_scene(scene){

}

void GridItem::setGridType(GridItem::GridType type) {
    m_gridType = type;
}

void GridItem::setCellSize(qreal size) {
    m_cellSize = size;
}

void GridItem::paint(QPainter *p, const QStyleOptionGraphicsItem *s, QWidget *w) {
    if (!isVisible()) return;

    QPen pen(Qt::gray, 0);
    pen.setCosmetic(true);
    p->setPen(pen);

    qreal pxPerFt = m_scene->getScaleFactor();
    qreal size = m_cellSize * pxPerFt;
    QRectF rect = m_scene->sceneRect();


    switch (m_gridType) {
        case Square:
            for (qreal x = std::floor(rect.left() / size) * size; x <= rect.right(); x += size)
                p->drawLine(QLineF(x, rect.top(), x, rect.bottom()));

            for (qreal y = std::floor(rect.top() / size) * size; y <= rect.bottom() ; y += size)
                p->drawLine(QLineF(rect.left(), y, rect.right(), y));
            break;
        case Hex:
            qreal dx = size * 3.0 / 4.0;
            qreal dy = size * std::sqrt(3) / 2.0;
            for (int row = 0;; ++row) {
                qreal y = rect.top() + row * dy;
                if (y > rect.bottom()) break;

                for (int col = 0;; ++col) {
                    qreal x = rect.left() + col * dx + (row % 2) * dx / 2;
                    if (x > rect.right()) break;

                    QPolygonF hex;
                    for (int i = 0; i < 6; ++i) {
                        qreal angle = M_PI / 3.0 * i;
                        hex << QPointF(x + size/2 * std::cos(angle), y + size/2 * std::sin(angle));
                    }
                    p->drawPolygon(hex);
                }
            }
    }

}
