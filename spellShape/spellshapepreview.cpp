#include "spellshapepreview.h"
#include <map-widget/mapscene.h>

SpellShapePreview::SpellShapePreview(QWidget *parent) : QGraphicsView(parent){

    m_scene = new QGraphicsScene(this);
}

void SpellShapePreview::setShape(SpellShapeType type, int feetSize, QColor color, QString textureName) {
    m_scene->clear();
    SpellShapeFactory::Params p;
    p.type = type;
    p.feetSize = feetSize;
    p.feetToPx = m_feetToPx;
    p.color = color;
    p.textureName = textureName;

    auto* item = SpellShapeFactory::createShape(m_scene, p);
    QRectF bounds = SpellShapeFactory::shapeBoudingRect(type, feetSize, m_feetToPx);

    fitInView(bounds.adjusted(-20, -20, 20, 20), Qt::KeepAspectRatio);

    delete m_gridItem;

    if (!m_gridItem){
        m_gridItem = new GridItem(bounds.adjusted(-20, -20, 20, 20));
        m_gridItem->setZValue(mapLayers::Grid);
        m_scene->addItem(m_gridItem);
        m_gridItem->setPixelsPerFoot(1/m_feetToPx);
        m_gridItem->setCellFeet(5);
        m_gridItem->setGridType(GridItem::GridType::Square);
        m_gridItem->setVisible(false);
    }
}
