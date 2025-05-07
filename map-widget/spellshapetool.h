#ifndef DM_ASSIST_SPELLSHAPETOOL_H
#define DM_ASSIST_SPELLSHAPETOOL_H

#include "abstractmaptool.h"
#include "spellshapeitem.h"

class SpellShapeTool : public AbstractMapTool {
Q_OBJECT
public:
    explicit SpellShapeTool(QObject *parent = nullptr);

    void mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) = 0;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) = 0;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override {};
    void wheelEvent(QGraphicsSceneWheelEvent *event, QGraphicsScene *scene) override {};
    void deactivate(QGraphicsScene *scene) { clearPreview(scene);};

    void setColor(QColor c);

protected:
    QColor color = Qt::cyan;
    QGraphicsItem *previewShape = nullptr;
    QGraphicsTextItem *previewLabel = nullptr;

    QPointF firstPoint;
    bool hasFirstPoint = false;

    void clearPreview(QGraphicsScene *scene);
    bool clearShapeAt(QGraphicsScene *scene, QPointF point);
};


class LineShapeTool : public SpellShapeTool {
Q_OBJECT
public:
    void mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene);
};

class CircleTool : public SpellShapeTool {
public:
    void mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override {
        if (!hasFirstPoint) {
            firstPoint = event->scenePos();
            hasFirstPoint = true;
        } else {
            QRectF rect(firstPoint, event->scenePos());
            scene->addEllipse(rect.normalized(), QPen(Qt::blue, 2), QBrush(Qt::NoBrush));
            hasFirstPoint = false;
            clearPreview(scene);
        }
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override {
        if (!hasFirstPoint) return;

        QRectF rect(firstPoint, event->scenePos());
        if (!previewShape) {
            previewShape = scene->addEllipse(rect.normalized(), QPen(Qt::DashLine));
        } else {
            static_cast<QGraphicsEllipseItem*>(previewShape)->setRect(rect.normalized());
        }

        double radius = QLineF(firstPoint, event->scenePos()).length();
        if (!previewLabel) {
            previewLabel = scene->addText(QString::number(radius, 'f', 1) + " ft");
        }
        previewLabel->setPos(rect.center() + QPointF(10, -10));
    }

    void mouseRightClickEvent(QGraphicsSceneMouseEvent *, QGraphicsScene *scene) override {
        hasFirstPoint = false;
        clearPreview(scene);
    }
};


class TriangleTool : public SpellShapeTool {
public:
    void mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override {
        if (!hasFirstPoint) {
            firstPoint = event->scenePos();
            hasFirstPoint = true;
        } else {
            QPointF p2 = event->scenePos();
            QPointF p3 = mirrorPoint(firstPoint, p2);
            QPolygonF triangle{ firstPoint, p2, p3 };
            scene->addPolygon(triangle, QPen(Qt::blue, 2));
            hasFirstPoint = false;
            clearPreview(scene);
        }
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override {
        if (!hasFirstPoint) return;

        QPointF p2 = event->scenePos();
        QPointF p3 = mirrorPoint(firstPoint, p2);
        QPolygonF triangle{ firstPoint, p2, p3 };

        if (!previewShape) {
            previewShape = scene->addPolygon(triangle, QPen(Qt::DashLine));
        } else {
            static_cast<QGraphicsPolygonItem*>(previewShape)->setPolygon(triangle);
        }

        double baseLength = QLineF(p2, p3).length();
        if (!previewLabel) {
            previewLabel = scene->addText(QString::number(baseLength, 'f', 1) + " ft");
        }
        QPointF center = (p2 + p3) / 2;
        previewLabel->setPos(center + QPointF(10, -10));
    }

    void mouseRightClickEvent(QGraphicsSceneMouseEvent *, QGraphicsScene *scene) override {
        hasFirstPoint = false;
        clearPreview(scene);
    }

private:
    QPointF mirrorPoint(const QPointF &center, const QPointF &p) {
        return center * 2 - p;
    }
};

class SquareTool : public SpellShapeTool {
public:
    void mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override {
        if (!hasFirstPoint) {
            firstPoint = event->scenePos();
            hasFirstPoint = true;
        } else {
            QPolygonF square = buildSquare(firstPoint, event->scenePos());
            scene->addPolygon(square, QPen(Qt::blue, 2));
            hasFirstPoint = false;
            clearPreview(scene);
        }
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene) override {
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

    void mouseRightClickEvent(QGraphicsSceneMouseEvent *, QGraphicsScene *scene) override {
        hasFirstPoint = false;
        clearPreview(scene);
    }

private:
    QPolygonF buildSquare(const QPointF &p1, const QPointF &p2) {
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
};


#endif //DM_ASSIST_SPELLSHAPETOOL_H
