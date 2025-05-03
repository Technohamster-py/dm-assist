//
// Created by arseniy on 02.05.2025.
//

#include "sharedmapwindow.h"
#include "mapscene.h"
#include <QVBoxLayout>

SharedMapWindow::SharedMapWindow(MapScene *originalScene, QWidget *parent)
    : QWidget(parent){
    scene = new MapScene(this);
    scene->setSceneRect(originalScene->sceneRect());

    scene->addPixmap(originalScene->getMapPixmap());

    // Создаём fogItem
    fogItem = new QGraphicsPixmapItem();
    fogItem->setZValue(10);
    fogItem->setOpacity(1.0); // полностью непрозрачно
    scene->addItem(fogItem);

    view = new QGraphicsView(scene, this);
    view->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setInteractive(false);
    view->setDragMode(QGraphicsView::NoDrag);

    QVBoxLayout* layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->addWidget(view);

    setAttribute(Qt::WA_DeleteOnClose);

    connect(originalScene, &MapScene::fogUpdated, this, &SharedMapWindow::updateFogImage);
}

void SharedMapWindow::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    if (view && view->scene())
        view->fitInView(view->scene()->sceneRect(), Qt::KeepAspectRatio);
}

void SharedMapWindow::updateFogImage(const QImage &fog) {
    fogItem->setPixmap(QPixmap::fromImage(fog));
}
