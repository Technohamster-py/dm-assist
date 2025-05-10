#include "spellshapetool.h"
#include "mapscene.h"

SpellShapeTool::SpellShapeTool(QObject *parent)
        : AbstractMapTool(parent) {
    color.setAlpha(180);
}

void SpellShapeTool::clearPreview(QGraphicsScene *scene) {
    if (previewShape) {
        scene->removeItem(previewShape);
        delete previewShape;
        previewShape = nullptr;
    }
    if (previewLabel) {
        scene->removeItem(previewLabel);
        delete previewLabel;
        previewLabel = nullptr;
    }
}

void SpellShapeTool::setColor(QColor c) {
    color = c;
    color.setAlpha(180);
}

bool SpellShapeTool::clearShapeAt(QGraphicsScene *scene, QPointF point) {
    QGraphicsItem *item = scene->itemAt(point, QTransform());
    if (item) {
        static_cast<MapScene*>(scene)->removeUndoableItem(item);
        return true;
    }
    return false;
}


void LineShapeTool::mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (!hasFirstPoint){
        if (event->button() == Qt::RightButton) {
            clearShapeAt(scene, event->scenePos());
            return;
        }

        firstPoint = event->scenePos();
        hasFirstPoint = true;
    } else {
        if (event->button() == Qt::RightButton){
            hasFirstPoint = false;
            clearPreview(scene);
            return;
        }

        QLineF line(firstPoint, event->scenePos());

        auto item = new QGraphicsLineItem(line);
        item->setPen(QPen(color, 2));
        item->setZValue(5);

        static_cast<MapScene*>(scene)->addUndoableItem(item);

        hasFirstPoint = false;
        clearPreview(scene);
    }
}

void LineShapeTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (!hasFirstPoint)
        return;

    QLineF line(firstPoint, event->scenePos());
    if (!previewShape) {
        previewShape = scene->addLine(line, QPen(Qt::DashLine));
    } else {
        dynamic_cast<QGraphicsLineItem*>(previewShape)->setLine(line);
    }

    double distance = 0.0;

    if (auto mapScene = qobject_cast<MapScene*>(scene)){
        distance = line.length() * mapScene->getScaleFactor();
    } else {
        distance = line.length();
    }

    if (!previewLabel) {
        previewLabel = scene->addText(QString("%1 ft").arg(distance, 0, 'f', 1));
    } else {
        previewLabel->setPlainText(QString("%1 ft").arg(distance, 0, 'f', 1));
    }
    previewLabel->setPos(line.pointAt(0.5) + QPointF(10, -10));
}

void CircleShapeTool::mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (!hasFirstPoint) {
        if (event->button() == Qt::RightButton) {
            clearShapeAt(scene, event->scenePos());
            return;
        }
        firstPoint = event->scenePos();
        hasFirstPoint = true;
    } else {
        if (event->button() == Qt::RightButton){
            hasFirstPoint = false;
            clearPreview(scene);
            return;
        }

        QRectF rect = circleRect(event->scenePos());

        auto item = new QGraphicsEllipseItem(rect.normalized());
        item->setPen(QPen(color));
        item->setBrush(QBrush(color, Qt::Dense4Pattern));
        item->setZValue(5);

        static_cast<MapScene*>(scene)->addUndoableItem(item);

        hasFirstPoint = false;
        clearPreview(scene);
    }
}

void CircleShapeTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (!hasFirstPoint) return;

    QRectF rect = circleRect(event->scenePos());
    if (!previewShape) {
        previewShape = scene->addEllipse(rect.normalized(), QPen(Qt::DashLine));
    } else {
        dynamic_cast<QGraphicsEllipseItem*>(previewShape)->setRect(rect.normalized());
    }

    double radius = QLineF(firstPoint, event->scenePos()).length();
    if (!previewLabel) {
        previewLabel = scene->addText(QString::number(radius, 'f', 1) + " ft");
    }
    previewLabel->setPos(rect.center() + QPointF(10, -10));
}

QRectF CircleShapeTool::circleRect(QPointF point) {
    double radius = QLineF(firstPoint, point).length();

    QPointF topLeft(firstPoint.x()-radius, firstPoint.y()-radius);
    QPointF bottomRight(firstPoint.x()+radius, firstPoint.y()+radius);

    return QRectF(topLeft, bottomRight);
}

void TriangleShapeTool::mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (!hasFirstPoint) {
        if (event->button() == Qt::RightButton) {
            clearShapeAt(scene, event->scenePos());
            return;
        }

        firstPoint = event->scenePos();
        hasFirstPoint = true;
    } else {
        if (event->button() == Qt::RightButton){
            hasFirstPoint = false;
            clearPreview(scene);
            return;
        }

        QPolygonF triangle = buildTriangle(event->scenePos());

        auto item = new QGraphicsPolygonItem(triangle);
        item->setPen(QPen(color));
        item->setBrush(QBrush(color, Qt::Dense4Pattern));
        item->setZValue(5);

        static_cast<MapScene*>(scene)->addUndoableItem(item);

        hasFirstPoint = false;
        clearPreview(scene);
    }
}

void TriangleShapeTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (!hasFirstPoint) return;

    QPointF p2 = event->scenePos();
    QPolygonF triangle = buildTriangle(p2);

    if (!previewShape) {
        previewShape = scene->addPolygon(triangle, QPen(Qt::DashLine));
    } else {
        dynamic_cast<QGraphicsPolygonItem*>(previewShape)->setPolygon(triangle);
    }

    double baseLength = QLineF(firstPoint, p2).length();
    if (!previewLabel) {
        previewLabel = scene->addText(QString::number(baseLength, 'f', 1) + " ft");
    }
    QPointF center = (firstPoint + p2) / 2;
    previewLabel->setPos(center + QPointF(10, -10));
}

QPolygonF TriangleShapeTool::buildTriangle(const QPointF &hPoint) {
    QPointF a = firstPoint;
    QPointF h = hPoint;

    // Вектор высоты
    QLineF ah(h, a);
    double height = ah.length();

    // Перпендикулярный вектор основания
    QLineF baseLine = ah.normalVector();
    baseLine.setLength(height);
    QPointF offset = (baseLine.p2() - baseLine.p1()) / 2;

    // Точки B и C
    QPointF b = h + offset;
    QPointF c = h - offset;

    QPolygonF triangle; triangle << a << b << c;
    return triangle;
}


void SquareShapeTool::mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (!hasFirstPoint) {
        if (event->button() == Qt::RightButton) {
            clearShapeAt(scene, event->scenePos());
            return;
        }

        firstPoint = event->scenePos();
        hasFirstPoint = true;
    } else {
        if (event->button() == Qt::RightButton) {
            clearShapeAt(scene, event->scenePos());
            return;
        }

        QPolygonF square = buildSquare(firstPoint, event->scenePos());

        auto item = new QGraphicsPolygonItem(square);
        item->setPen(QPen(color));
        item->setBrush(QBrush(color, Qt::Dense4Pattern));
        item->setZValue(5);

        static_cast<MapScene*>(scene)->addUndoableItem(item);
        hasFirstPoint = false;
        clearPreview(scene);
    }
}

void SquareShapeTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) {
    if (!hasFirstPoint) return;

    QPolygonF square = buildSquare(firstPoint, event->scenePos());
    if (!previewShape) {
        previewShape = scene->addPolygon(square, QPen(Qt::DashLine));
    } else {
        static_cast<QGraphicsPolygonItem*>(previewShape)->setPolygon(square);
    }

    double diagLength = QLineF(firstPoint, event->scenePos()).length();
    if (!previewLabel) {
        previewLabel = scene->addText(QString::number(diagLength, 'f', 1) + " ft");
    }
    QPointF center = (firstPoint + event->scenePos()) / 2;
    previewLabel->setPos(center + QPointF(10, -10));
}

QPolygonF SquareShapeTool::buildSquare(const QPointF &p1, const QPointF &p2) {
    QLineF diag(p1, p2);
    QPointF center = p1 + (p2 - p1) * 0.5;

    QLineF side1 = diag.normalVector(); side1.setLength(diag.length());
    QPointF v1 = side1.p2() - side1.p1();

    QLineF side2 = diag; side2.setLength(diag.length());
    QPointF v2 = side2.p2() - side2.p1();

    QPointF a = center - v1 / 2 - v2 / 2;
    QPointF b = center + v1 / 2 - v2 / 2;
    QPointF c = center + v1 / 2 + v2 / 2;
    QPointF d = center - v1 / 2 + v2 / 2;

    QPolygonF square; square << a << b << c << d;
    return square;
}

