#ifndef DM_ASSIST_SHAREDMAPWINDOW_H
#define DM_ASSIST_SHAREDMAPWINDOW_H

#include <QGraphicsView>
#include "mapscene.h"


class SharedMapWindow : public QWidget{
Q_OBJECT
public:
    explicit SharedMapWindow(MapScene *originalScene, QWidget *parent = nullptr);
public slots:
    void updateFogImage(const QImage &fog);
private:
    QGraphicsView *view;
    MapScene *scene;
    QGraphicsPixmapItem *fogItem;
protected:
    void resizeEvent(QResizeEvent* event) override;
};


#endif //DM_ASSIST_SHAREDMAPWINDOW_H
