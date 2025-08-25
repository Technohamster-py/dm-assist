#ifndef DM_ASSIST_GRIDITEM_H
#define DM_ASSIST_GRIDITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include "mapscene.h"

class GridItem : public QGraphicsItem{
public:
    enum GridType {Square, Hex};

    explicit GridItem(MapScene* scene);
    void setGridType(GridType type);
    void setCellSize(qreal size);
    void setVisible(bool visible);

    QRectF boundingRect() const override;
    void paint(QPainter* p, const QStyleOptionGraphicsItem* s, QWidget* w) override;

private:
    GridType m_gridType = Square;
    qreal m_cellSize = 5.0; ///< ft
    MapScene* m_scene;
};


#endif //DM_ASSIST_GRIDITEM_H
