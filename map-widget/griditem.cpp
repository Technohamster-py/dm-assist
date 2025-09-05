#include "griditem.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>

GridItem::GridItem(const QRectF &size, QGraphicsItem *parent) : QGraphicsItem(parent), m_rect(size)
{
    m_pen.setCosmetic(true);
}

void GridItem::paint(QPainter *p, const QStyleOptionGraphicsItem *opt, QWidget *)
{
    if (!isVisible() || m_cellFeet <= 0 || m_pixelsPerFoot <= 0) return;

    p->setRenderHint(QPainter::Antialiasing, false);
    p->setPen(m_pen);

    const QRectF exposed = opt ? opt->exposedRect : QRectF(-5e5,-5e5,1e6,1e6);

    qreal stepPx;
    switch (m_type) {
        case GridType::Square:
            stepPx = m_cellFeet * m_pixelsPerFoot;
            paintSquareGrid(p, exposed, stepPx);
            break;
        case GridType::Hex:
            stepPx = m_cellFeet * m_pixelsPerFoot; // side-to-side
            paintHexGrid(p, exposed, stepPx);
            break;
    }
}

void GridItem::paintSquareGrid(QPainter* p, const QRectF& rect, qreal stepPx)
{
    if (stepPx <= 0) return;

    qreal x0 = std::floor(rect.left() / stepPx) * stepPx;
    qreal x1 = rect.right();
    qreal y0 = std::floor(rect.top() / stepPx) * stepPx;
    qreal y1 = rect.bottom();

    for (qreal x = x0; x <= x1; x += stepPx)
        p->drawLine(QLineF(x, rect.top(), x, rect.bottom()));

    for (qreal y = y0; y <= y1; y += stepPx)
        p->drawLine(QLineF(rect.left(), y, rect.right(), y));
}



void GridItem::paintHexGrid(QPainter* p, const QRectF& rect, qreal flatToFlatPx)
{
    if (flatToFlatPx <= 0) return;

    const qreal s  = flatToFlatPx / std::sqrt(3.0);
    const qreal h  = 2.0 * s;
    const qreal dx = flatToFlatPx;
    const qreal dy = 0.75 * h;

    QPolygonF hex;
    for (int i = 0; i < 6; ++i) {
        qreal ang = M_PI/3.0 * i - M_PI/6.0;
        hex << QPointF(std::cos(ang) * s, std::sin(ang) * s);
    }

    int rowStart = int(std::floor(rect.top() / dy)) - 1;
    int rowEnd   = int(std::ceil (rect.bottom() / dy)) + 1;

    for (int row = rowStart; row <= rowEnd; ++row) {
        qreal cy = row * dy;
        if (cy > rect.bottom()) break;

        qreal offsetX = (row & 1) ? dx * 0.5 : 0.0;

        int colStart = int(std::floor((rect.left() - offsetX) / dx)) - 1;
        int colEnd   = int(std::ceil ((rect.right()- offsetX) / dx)) + 1;

        for (int col = colStart; col <= colEnd; ++col) {
            qreal cx = offsetX + col * dx;
            if (cx > rect.right()) break;

            QTransform t = QTransform::fromTranslate(cx, cy);
            p->drawPolygon(t.map(hex));
        }
    }
}

QString GridItem::stringMode(int mode) {
    switch (mode) {
        case GridType::None: return QObject::tr("None");
        case GridType::Square: return QObject::tr("Square");
        case GridType::Hex: return QObject::tr("Hexagon");
    }
}

