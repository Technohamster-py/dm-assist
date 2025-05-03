//
// Created by arseniy on 02.05.2025.
//

#include "sharedmapwindow.h"
#include "mapscene.h"
#include <QVBoxLayout>

SharedMapWindow::SharedMapWindow(QGraphicsScene *scene, QWidget *parent)
    : QWidget(parent){
    view = new QGraphicsView(scene, this);
    view->setDragMode(QGraphicsView::NoDrag);
    view->setInteractive(false);
    view->setTransformationAnchor(QGraphicsView::AnchorViewCenter);

    QVBoxLayout* layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->addWidget(view);

    setAttribute(Qt::WA_DeleteOnClose);

    // Установим непрозрачность тумана
    auto mapScene = dynamic_cast<MapScene*>(scene);
    if (mapScene) {
        mapScene->setFogOpacity(1.0); // полностью непрозрачно
    }
}

void SharedMapWindow::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    if (view && view->scene())
        view->fitInView(view->scene()->sceneRect(), Qt::KeepAspectRatio);
}
