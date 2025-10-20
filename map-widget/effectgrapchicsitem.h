#ifndef DM_ASSIST_EFFECTGRAPCHICSITEM_H
#define DM_ASSIST_EFFECTGRAPCHICSITEM_H

#include <QGraphicsItem>

class EffectGraphicsItem{
public:
    ~EffectGraphicsItem() = default;

    virtual QString textureFileName() const {return m_textureName;};
    virtual void setTexture(const QString& textureFilePath) {
        m_textureName = textureFilePath;
        updateBrush();
    };

    virtual QColor color() const {return m_color;};
    virtual void setColor(const QColor& c) {
        m_color = c;
        updateBrush();
    };

    virtual QJsonObject toJson() const = 0;
    virtual void fromJson(const QJsonObject& obj) = 0;

protected:
    virtual void updateBrush() = 0;

    QString m_textureName;
    QColor m_color = Qt::cyan;
};


class EffectPolygonItem : public QGraphicsPolygonItem, public EffectGraphicsItem {
public:
    EffectPolygonItem(const QPolygonF& polygon = {}, const QColor& color = Qt::red, const QString& texture = {}, QGraphicsItem* parent = nullptr);
    void updateBrush() override;
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& obj) override;
};


class EffectEllipseItem : public QGraphicsEllipseItem, public EffectGraphicsItem {
public:
    EffectEllipseItem(const QRectF& rect = {}, const QColor& color = Qt::green, const QString& texture = {}, QGraphicsItem* parent = nullptr);
    void updateBrush() override;
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& obj) override;
};


#endif //DM_ASSIST_EFFECTGRAPCHICSITEM_H
