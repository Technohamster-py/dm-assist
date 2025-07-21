/**
 * @file mapscene.h
 * @brief MapScene manages map elements and tool interactions in the RPG map viewer.
 */

#ifndef MAPSCENE_H
#define MAPSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <QJsonObject>

#include "abstractmaptool.h"
#include "undostack.h"


#pragma pack(push, 1)
struct MapFileHeader {
    quint32 magic = 0x444D414D; // 'DMAM'
    quint32 version = 1;
    quint32 jsonSize;
    quint32 imageSize;
};
#pragma pack(pop)

enum mapErrorCodes{
    NoError,
    FileOpenError,
    FileSignatureError,
    JsonParseError
};


enum mapLayers{
    Background = -100,
    Height = 3,
    Shapes = 5,
    Brush = 6,
    Ruler = 10,
    Fog = 100,
    Light = 110
};


/**
 * @class MapScene
 * @brief Represents a custom scene for displaying and interacting with a map, supporting tools, fog of war, and undoable actions.
 *
 * The MapScene class extends QGraphicsScene to provide functionalities like tool management,
 * fog of war manipulation, and undo/redo capabilities for certain scene modifications. It emits
 * relevant signals when tools are changed or the fog of war is updated.
 */
class MapScene : public QGraphicsScene
{
Q_OBJECT

public:
    explicit MapScene(QObject *parent = nullptr);
    void setActiveTool(AbstractMapTool* tool);

    void setScaleFactor(double factor);
    [[nodiscard]] double getScaleFactor() const {return m_scaleFactor;};

    void initializeFog(const QSize &size);
    void drawFogCircle(const QPointF &scenePos, int radius, bool hide);
    void drawScaledCircle(const QPointF &scenePos, int radius, bool hide);
    void drawFogPath(const QPainterPath &path, bool hide);
    void clearFog();
    QImage getFogImage() const {return fogImage;};
    QPixmap getMapPixmap() const;
    QRectF mapRect() const;
    void setFogOpacity(qreal opacity); // 0.0–1.0
    void updateFog();

    QJsonObject toJson();
    void fromJson(const QJsonObject& obj);
    bool saveToFile(const QString& path);
    int loadFromFile(const QString& path);


    void wheelEvent(QGraphicsSceneWheelEvent *event) override;

    void addUndoableItem(QGraphicsItem* item);
    void removeUndoableItem(QGraphicsItem* item);
    void undoLastAction();

    qreal heightAt(const QPointF& pos) const;
signals:
    void fogUpdated(const QImage &fogImage);
    void toolChanged(const AbstractMapTool*);

private:
    AbstractMapTool* m_activeTool = nullptr;
    double m_scaleFactor = 1.0;           ///< Масштаб

    QGraphicsPixmapItem *fogItem = nullptr;
    QImage fogImage;

    UndoStack undoStack;
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // MAPSCENE_H
