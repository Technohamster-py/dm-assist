#include "griditem.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>

GridItem::GridItem(const QRectF &size, QGraphicsItem *parent) : QGraphicsItem(parent), m_rect(size)
{
    m_pen.setCosmetic(true);
}

/**
 * @brief Renders the grid on the scene using a specified grid type.
 *
 * This function handles the painting of grid lines within the exposed rectangular area.
 * The type of grid to draw (square or hexagonal) is determined by the member variable `m_type`.
 * Ensures that a grid is only drawn if the item is visible and the scaling factors
 * (`m_cellFeet` and `m_pixelsPerFoot`) are valid.
 *
 * @param p Pointer to a QPainter instance used for rendering the grid.
 * @param opt Pointer to a QStyleOptionGraphicsItem providing details about rendering options,
 *            such as the exposed rectangle.
 * @param w Unused widget parameter, typically set to nullptr.
 */
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

/**
 * @brief Draws a square grid within the specified rectangular area.
 *
 * This function renders a square grid with lines separated by a fixed
 * pixel distance (`stepPx`) inside the given rectangular region (`rect`).
 * Horizontal and vertical lines are drawn iteratively to create the grid.
 * This method is utilized when the grid type is set to Square.
 *
 * @param p Pointer to the QPainter object used for rendering the grid lines.
 * @param rect The rectangular region within which the grid is drawn.
 * @param stepPx The distance between adjacent grid lines in pixels.
 *               Must be greater than zero.
 */
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


/**
 * @brief Paints a hexagonal grid on the provided rectangular area.
 *
 * This function draws a grid of hexagons within the specified rectangular area.
 * The spacing and size of the hexagons are determined by the `flatToFlatPx` parameter,
 * which represents the flat-to-flat (horizontal) distance of a hexagon in pixels.
 * Hexagons are positioned in staggered rows where every second row is horizontally
 * offset to create a honeycomb pattern.
 *
 * @param p Pointer to a QPainter instance used for rendering the hexagonal grid.
 * @param rect Specifies the rectangular area to be covered by the hexagonal grid.
 * @param flatToFlatPx The flat-to-flat distance of a hexagon in pixels. Must be greater than 0.
 */
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

/**
 * @brief Converts a grid type mode to its corresponding string representation.
 *
 * This static function accepts an integer value representing a grid type
 * and returns the localized string representation of that type.
 * It supports `None`, `Square`, and `Hex` grid types as defined in the `GridType` enum.
 * If an unrecognized mode value is provided, it defaults to returning "None".
 *
 * @param mode Integer value representing the grid type.
 *             Valid values align with `GridType` enum: `None`, `Square`, `Hex`.
 * @return QString Localized string representation of the grid type.
 */
QString GridItem::stringMode(int mode) {
    switch (mode) {
        case GridType::None: return QObject::tr("None");
        case GridType::Square: return QObject::tr("Square");
        case GridType::Hex: return QObject::tr("Hexagon");
        default: return QObject::tr("None");
    }
}

