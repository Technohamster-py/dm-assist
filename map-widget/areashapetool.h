#ifndef DM_ASSIST_AREASHAPETOOL_H
#define DM_ASSIST_AREASHAPETOOL_H

#include "abstractmaptool.h"

class AreaShapeTool : public AbstractMapTool{
Q_OBJECT
public:
    explicit AreaShapeTool(QObject* parent = nullptr);

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override = 0;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override {};
    void wheelEvent(QGraphicsSceneWheelEvent *event, QGraphicsScene *scene) override {};
    void deactivate(QGraphicsScene *scene) override { clearPreview(scene);};

    void setColor(QColor c);
    void setTexture(const QString& textureName = "") {currentTextureName = textureName;};

protected:
    QColor color = Qt::cyan;
    QString currentTextureName = "fire";
    QGraphicsItem *previewShape = nullptr;
    QGraphicsTextItem *previewLabel = nullptr;

    void clearPreview(QGraphicsScene *scene);
    static bool clearShapeAt(QGraphicsScene *scene, QPointF point);
    QBrush getBrush();
};


#endif //DM_ASSIST_AREASHAPETOOL_H
