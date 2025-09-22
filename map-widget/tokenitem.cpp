#include "tokenitem.h"
#include "mapscene.h"
#include "abstractcharsheetwidget.h"
#include "fvttparser.h"
#include "charaсterparser.h"
#include <QGraphicsSceneContextMenuEvent>
#include <QPen>
#include <QMenu>
#include <QUrl>

TokenItem::TokenItem(const QString& name, const QPixmap& pixmap, qreal realSize, qreal pxPerFoot)
        : m_realSize(realSize), m_pxPerFoot(pxPerFoot)
{
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsScenePositionChanges);

    qreal sizePx = realSize * pxPerFoot;

    pixmapItem = new QGraphicsPixmapItem(pixmap.scaled(sizePx, sizePx, Qt::KeepAspectRatio, Qt::SmoothTransformation), this);
    pixmapItem->setOffset(-pixmapItem->boundingRect().width()/2, -pixmapItem->boundingRect().height()/2);

    labelItem = new QGraphicsSimpleTextItem(name, this);
    labelItem->setBrush(Qt::white);
    labelItem->setPen(QPen(Qt::black, 2));
    labelItem->setPos(-labelItem->boundingRect().width()/2, pixmapItem->boundingRect().height()/2);
}

void TokenItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QMenu menu;
    QAction* tracker = menu.addAction("Add to tracker");
    QAction* sheet   = menu.addAction("Open charsheet");

    QAction* selected = menu.exec(event->screenPos());
    if (selected == tracker) emit addToTracker(this);
    else if (selected == sheet) emit openCharSheet(this);
}

void TokenItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsObject::mouseReleaseEvent(event);

    // снап к сетке
    if (scene()) {
        auto* mapScene = qobject_cast<MapScene*>(scene());
        if (mapScene && mapScene->gridEnabled()) {
            qreal step = 5 * m_pxPerFoot; // базовый размер клетки
            QPointF pos = this->pos();
            QPointF snapped(qRound(pos.x()/step)*step, qRound(pos.y()/step)*step);
            setPos(snapped);
        }
    }
}

TokenStruct TokenItem::fromJson(const QString &filePath) {
    QString campaignPath = AbstractCharsheetWidget::campaignDirFromFile(filePath);

    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) return {};

    TokenStruct tokenStruct;
    QJsonObject obj = QJsonDocument::fromJson(f.readAll()).object();
    if (obj["jsonType"].isNull()){
        BestiaryPageData data;
        IFvttParser* parser;
        if (!obj["system"].isNull())
            parser = new Fvtt11Parser;
        else
            parser = new Fvtt10Parser;
        data = parser->parse(obj);
        delete parser;

        tokenStruct.name = data.name;
        QString fullPath = AbstractCharsheetWidget::getTokenFileName(campaignPath, data.imgLink);
        tokenStruct.imgPath = fullPath;
        return tokenStruct;
    }
    else if (obj["jsonType"] == "character"){
        IDndParser* characterParser;

        characterParser = new LssDndParser;

        DndCharacterData data = characterParser->parseDnd(filePath);

        tokenStruct.name = data.name;
        QString fullPath = AbstractCharsheetWidget::getTokenFileName(campaignPath, data.tokenUrl);
        tokenStruct.imgPath = fullPath;
        return tokenStruct;
    }
    return {};
}
