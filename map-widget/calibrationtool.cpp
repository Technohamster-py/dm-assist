#include "calibrationtool.h"
#include "mapscene.h"
#include <QInputDialog>
#include <QLineF>
#include <QGraphicsSceneMouseEvent>
#include <QPen>

CalibrationTool::CalibrationTool(QObject *parent)
        : AbstractMapTool(parent) {}

void CalibrationTool::mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    QPointF pos = event->scenePos();

    if (event->button() != Qt::LeftButton)
        return;

    points.append(pos);

    if (points.size() == 2) {
        QLineF line(points[0], points[1]);
        double pixelDistance = line.length();

        bool ok;
        double realDistance = QInputDialog::getDouble(nullptr, tr("Scale calibration"),
                                                      tr("Enter distance in feet"),
                                                      5.0, 0.01, 10000, 2, &ok);
        if (ok && pixelDistance > 0.0) {
            double scale = realDistance / pixelDistance;
            auto mapScene = dynamic_cast<MapScene*>(scene);
            if (mapScene) {
                mapScene->setScaleFactor(scale);
            }
        }

        if (previewLine) {
            scene->removeItem(previewLine);
            delete previewLine;
            previewLine = nullptr;
        }

        points.clear();
        emit finished();
    }
}

void CalibrationTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (points.size() == 1) {
        QPointF current = event->scenePos();
        QLineF line(points[0], current);

        if (!previewLine) {
            previewLine = scene->addLine(line, QPen(Qt::DashLine));
        } else {
            previewLine->setLine(line);
        }
    }
}

void CalibrationTool::deactivate(QGraphicsScene *scene) {
    points.clear();
    if (previewLine) {
        scene->removeItem(previewLine);
        delete previewLine;
        previewLine = nullptr;
    }
}