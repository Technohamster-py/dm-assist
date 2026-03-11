#ifndef DM_ASSIST_HEIGHTMAPTOOL_H
#define DM_ASSIST_HEIGHTMAPTOOL_H

#include "areashapetool.h"
#include "mapscene.h"

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

class HeightMapTool : public LassoTool {
public:
    void mousePressEvent(QGraphicsSceneMouseEvent* event, QGraphicsScene* scene) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override;
};


#endif //DM_ASSIST_HEIGHTMAPTOOL_H
