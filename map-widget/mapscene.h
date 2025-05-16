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

enum qmapErrorCodes{
    NoError,
    FileOpenError,
    FileSignatureError,
    JsonParseError
};

/**
 * @class MapScene
 * @brief A scene for displaying and interacting with RPG map elements.
 */
class MapScene : public QGraphicsScene
{
Q_OBJECT

public:
    /**
     * @brief Constructs a new MapScene.
     * @param parent Optional parent QObject
     */
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
    void setFogOpacity(qreal opacity); // 0.0–1.0
    void updateFog();

    QJsonObject toJson() const;
    void fromJson(const QJsonObject& obj);
    bool saveToFile(const QString& path);
    int loadFromFile(const QString& path);


    void wheelEvent(QGraphicsSceneWheelEvent *event) override;

    void addUndoableItem(QGraphicsItem* item);
    void removeUndoableItem(QGraphicsItem* item);
    void undoLastAction();
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
