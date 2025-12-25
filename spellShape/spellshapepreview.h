#ifndef DM_ASSIST_SPELLSHAPEPREVIEW_H
#define DM_ASSIST_SPELLSHAPEPREVIEW_H

#include <QGraphicsView>
#include "spellshapefactory.h"
#include "../map-widget/griditem.h"

class SpellShapePreview : QGraphicsView{
Q_OBJECT
public:
    explicit SpellShapePreview(QWidget* parent = nullptr);

    void setShape(SpellShapeType type, int feetSize, QColor color, QString textureName);
    void setScaleFactor(qreal feetToPx) {m_feetToPx = feetToPx;};

private:
    QGraphicsScene* m_scene;
    GridItem* m_gridItem;
    QGraphicsItem* m_currentShape = nullptr;

    qreal m_feetToPx = 20.0;
};


#endif //DM_ASSIST_SPELLSHAPEPREVIEW_H
