#ifndef DM_ASSIST_SHAREDMAPWINDOW_H
#define DM_ASSIST_SHAREDMAPWINDOW_H

#include <QGraphicsView>
#include "mapscene.h"


/**
 * @class SharedMapWindow
 * @brief A window that provides a synchronized view of a shared map scene.
 *
 * The SharedMapWindow class displays a non-interactive view of a MapScene.
 * It manages a graphical view to render the map and updates the displayed fog of war image when changes occur.
 * The class scales the view to fit the available window size while maintaining the aspect ratio of the map.
 *
 * This class utilizes QGraphicsView and QGraphicsPixmapItem for rendering purposes.
 */
class SharedMapWindow : public QWidget{
Q_OBJECT
public:
    explicit SharedMapWindow(MapScene *originalScene, QWidget *parent = nullptr);
public slots:
    void updateFogImage(const QImage &fog);
private:
    QGraphicsView *view;
    QGraphicsPixmapItem *fogItem;
protected:
    void resizeEvent(QResizeEvent* event) override;
};


#endif //DM_ASSIST_SHAREDMAPWINDOW_H
