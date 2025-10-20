#include "effectgrapchicsitem.h"
#include <QBrush>
#include <QPen>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValueRef>

EffectPolygonItem::EffectPolygonItem(const QPolygonF &polygon, const QColor &color, const QString &texture,
                                     QGraphicsItem *parent) : QGraphicsPolygonItem(polygon, parent){
    m_color = color;
    m_textureName = texture;
    updateBrush();
}

void EffectPolygonItem::updateBrush() {
    QPixmap pixmap(m_textureName);
    if (!pixmap.isNull()){
        setBrush(QBrush(pixmap));
    } else {
        setBrush(QBrush(m_color, Qt::Dense4Pattern));
    }
    setPen(QPen(m_color, 0));
}

QJsonObject EffectPolygonItem::toJson() const {
    QJsonObject obj;

    obj["type"] = "effect_polygon";
    obj["color"] = m_color.name(QColor::HexArgb);
    obj["texture"] = m_textureName;
    QJsonArray points;
    for (auto& p : polygon()) {
        points.append(QJsonArray{p.x(), p.y()});
    }
    obj["points"] = points;
    obj["z"] = zValue();
    return obj;
}

void EffectPolygonItem::fromJson(const QJsonObject &obj) {
    m_color = QColor(obj["color"].toString());
    m_textureName = obj["texture"].toString();
    QPolygonF polygon;
    for (QJsonValueRef v : obj["points"].toArray()) {
        auto arr = v.toArray();
        polygon << QPointF(arr[0].toDouble(), arr[1].toDouble());
    }
    setZValue(obj["z"].toDouble());
    setPolygon(polygon);
    updateBrush();
}

EffectEllipseItem::EffectEllipseItem(const QRectF &rect, const QColor &color, const QString &texture,
                                     QGraphicsItem *parent): QGraphicsEllipseItem(rect, parent) {
    m_color = color;
    m_textureName = texture;
    updateBrush();
}

void EffectEllipseItem::updateBrush() {
    QPixmap pixmap(m_textureName);
    if (!pixmap.isNull()){
        setBrush(QBrush(pixmap));
    } else {
        setBrush(QBrush(m_color, Qt::Dense4Pattern));
    }
    setPen(QPen(m_color, 0));
}

QJsonObject EffectEllipseItem::toJson() const {
    QJsonObject obj;

    obj["type"] = "effect_ellipse";
    obj["rect"] = QJsonArray{rect().x(), rect().y(), rect().width(), rect().height()};
    obj["color"] = m_color.name(QColor::HexArgb);
    obj["texture"] = m_textureName;
    obj["z"] = zValue();

    return obj;
}

void EffectEllipseItem::fromJson(const QJsonObject &obj) {
    m_color = QColor(obj["color"].toString());
    m_textureName = obj["texture"].toString();
    auto arr = obj["rect"].toArray();
    setRect(arr[0].toDouble(), arr[1].toDouble(), arr[2].toDouble(), arr[3].toDouble());
    updateBrush();
}
