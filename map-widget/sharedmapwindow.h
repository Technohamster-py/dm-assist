#ifndef DM_ASSIST_SHAREDMAPWINDOW_H
#define DM_ASSIST_SHAREDMAPWINDOW_H

#include <QGraphicsView>

class SharedMapWindow : public QWidget{
Q_OBJECT
public:
    explicit SharedMapWindow(QGraphicsScene *scene, QWidget *parent = nullptr);
private:
    QGraphicsView *view;
protected:
    void resizeEvent(QResizeEvent* event) override;
};


#endif //DM_ASSIST_SHAREDMAPWINDOW_H
