#ifndef DM_ASSIST_TOKENITEM_H
#define DM_ASSIST_TOKENITEM_H

#include <QGraphicsObject>
#include <QJsonObject>
#include "fixedsizetextitem.h"

enum TokenTitleDisplayMode{
    always,
    tooltip,
    noTitle
};

struct TokenStruct{
    QString name = "";
    QString imgPath = "";
    qreal size = 5.0;
};

class TokenItem : public QGraphicsObject
{
    Q_OBJECT
public:
    TokenItem(const QString &filePath, const QString &name, const QPixmap &pixmap, qreal realSize, qreal pxPerFoot);

    QRectF boundingRect() const override { return childrenBoundingRect(); }
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override {}
    void setGridStep(int step);
    int gridStep() const {return m_gridStep;}
    void setTitleDisplayMode(int mode);
    void setFontSize(int size);
    void setRealSize(qreal size);
    int mode() const {return m_mode;}

    static TokenStruct fromJson(const QString &filePath);
    static QString stringMode(int mode);
signals:
    void addToTracker(const QString&);
    void openCharSheet(const QString&);

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    QPixmap originalPixmap;
    QGraphicsPixmapItem* pixmapItem;
    FixedSizeTextItem* labelItem;
    qreal m_realSize;
    qreal m_pxPerFoot;
    QString m_filePath;
    int m_gridStep = 5;
    int m_mode = TokenTitleDisplayMode::always;
};



#endif //DM_ASSIST_TOKENITEM_H
