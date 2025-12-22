#ifndef DM_ASSIST_SHAREDMAPWINDOW_H
#define DM_ASSIST_SHAREDMAPWINDOW_H

#include <QGraphicsView>
#include "mapscene.h"



class SharedMapView : public QGraphicsView {
    Q_OBJECT
public:
    explicit SharedMapView(MapScene* scene, QWidget* parent = nullptr) : QGraphicsView(parent), m_scene(scene) { setScene(scene);}

    void updateMap(MapScene* scene) {
        m_scene=scene;
        setScene(scene);
    };
    void setOpacity(qreal opacity) {m_opacity = opacity;}
    qreal opacity() const {return m_opacity;}
protected:
    void drawForeground(QPainter* painter, const QRectF& rect) override {
        Q_UNUSED(rect);
        if (!m_scene) return;

        if (!m_scene->getFogImage().isNull()){
            painter->save();
            painter->setOpacity(m_opacity);
            painter->drawImage(QPointF(0, 0), m_scene->getFogImage());
            painter->restore();
        }
    }
private:
    MapScene *m_scene;
    qreal m_opacity = 1.0;
};

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
    void setFogOpacity(qreal opacity) {view->setOpacity(opacity);}
    void changeMap(MapScene* newScene);
public slots:
    void updateFogImage(const QImage &fog);
private:
    SharedMapView *view;
    QGraphicsPixmapItem *fogItem;
protected:
    void resizeEvent(QResizeEvent* event) override;
};


#endif //DM_ASSIST_SHAREDMAPWINDOW_H
