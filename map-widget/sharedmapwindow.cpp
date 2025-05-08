//
// Created by arseniy on 02.05.2025.
//

#include "sharedmapwindow.h"
#include "mapscene.h"
#include <QVBoxLayout>

SharedMapWindow::SharedMapWindow(MapScene *originalScene, QWidget *parent)
    : QWidget(parent){
    view = new QGraphicsView(originalScene, this);
    view->setDragMode(QGraphicsView::NoDrag);
    view->setInteractive(false);
    view->setTransformationAnchor(QGraphicsView::AnchorViewCenter);

    view->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    view->fitInView(originalScene->sceneRect(), Qt::KeepAspectRatio);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QVBoxLayout* layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->addWidget(view);

    setAttribute(Qt::WA_DeleteOnClose);

//    connect(originalScene, &MapScene::fogUpdated, this, &SharedMapWindow::updateFogImage);
//    updateFogImage(originalScene->getFogImage());
}

void SharedMapWindow::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    if (view && view->scene())
        view->fitInView(view->scene()->sceneRect(), Qt::KeepAspectRatio);
}

void SharedMapWindow::updateFogImage(const QImage &fog) {
    fogItem->setPixmap(QPixmap::fromImage(fog));
}
