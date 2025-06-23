#include "sharedmapwindow.h"
#include "mapscene.h"
#include <QVBoxLayout>

/**
 * Constructor of SharedMapWindow.
 *
 * This function initializes the SharedMapWindow instance with the specified parent widget.
 * It creates a QGraphicsView instance and sets its properties to match the desired behavior.
 * It also sets up the layout and connects the scene's fogUpdated signal to the updateFogImage
 * slot.
 *
 * Parameters:
 * - originalScene : A pointer to the MapScene instance to be displayed in the view.
 * - parent : A pointer to the parent widget for the SharedMapWindow instance.
 *
 * Notes:
 * - The QGraphicsView is configured with the following settings:
 *   - No user interaction (dragging or modifying content is disabled).
 *   - A center anchor for all transformations and fit-to-view scaling mode.
 *   - Antialiasing and smooth pixmap transformations are enabled for better rendering quality.
 *   - Scrollbars are disabled, as the content is meant to fit within the view bounds.
 */
SharedMapWindow::SharedMapWindow(MapScene *originalScene, QWidget *parent)
    : QWidget(parent){
    view = new QGraphicsView(originalScene, this);
    view->setDragMode(QGraphicsView::NoDrag);
    view->setInteractive(false);
    view->setTransformationAnchor(QGraphicsView::AnchorViewCenter);

    view->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    view->fitInView(originalScene->mapRect(), Qt::KeepAspectRatio);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto* layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->addWidget(view);

    setAttribute(Qt::WA_DeleteOnClose);

//    connect(originalScene, &MapScene::fogUpdated, this, &SharedMapWindow::updateFogImage);
//    updateFogImage(originalScene->getFogImage());
}

/**
 * Handles the resize event for the SharedMapWindow.
 *
 * This function overrides the base class's resizeEvent to provide additional
 * functionality. Upon resizing, if the associated view and its scene are valid,
 * the viewport of the QGraphicsView is adjusted to fit the dimensions of
 * the scene while maintaining the aspect ratio.
 *
 * @param event A pointer to the QResizeEvent object containing information
 * about the resize operation.
 */
void SharedMapWindow::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    if (view && view->scene())
        view->fitInView(view->scene()->sceneRect(), Qt::KeepAspectRatio);
}

void SharedMapWindow::updateFogImage(const QImage &fog) {
    fogItem->setPixmap(QPixmap::fromImage(fog));
}
