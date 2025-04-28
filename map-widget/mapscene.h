/**
 * @file mapscene.h
 * @brief MapScene manages map elements and tool interactions in the RPG map viewer.
 */

#ifndef MAPSCENE_H
#define MAPSCENE_H

#include <QGraphicsScene>

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

    // Инструменты и логика будут добавлены позже
};

#endif // MAPSCENE_H
