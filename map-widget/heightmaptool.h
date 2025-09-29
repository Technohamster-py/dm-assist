#ifndef DM_ASSIST_HEIGHTMAPTOOL_H
#define DM_ASSIST_HEIGHTMAPTOOL_H

#include "abstractmaptool.h"
#include "mapscene.h"
#include <QPainterPath>
#include <QGraphicsPathItem>


class HeightRegionItem : public QGraphicsPolygonItem {
public:
    HeightRegionItem(const QPolygonF& poly, qreal height);

    qreal height() const {return m_height;};
    void setHeight(qreal h);
    static QColor heightToColor(qreal h);
private:
    qreal m_height = 0;
    void updateColor();
};

class HeightMapTool : public  AbstractMapTool{
public:
    void mousePressEvent(QGraphicsSceneMouseEvent* event, QGraphicsScene* scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
    void wheelEvent(QGraphicsSceneWheelEvent *event, QGraphicsScene *scene) override {};
    void deactivate(QGraphicsScene *scene) override {};
//    void cancel();
private:
    QPainterPath path;
    QGraphicsPathItem* preview = nullptr;
};


#endif //DM_ASSIST_HEIGHTMAPTOOL_H
