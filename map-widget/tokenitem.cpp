#include "tokenitem.h"
#include "mapscene.h"
#include "abstractcharsheetwidget.h"
#include "fvttparser.h"
#include "characterparser.h"
#include <QGraphicsSceneContextMenuEvent>
#include <QPen>
#include <QMenu>
#include <QUrl>
#include <QInputDialog>

TokenItem::TokenItem(const QString &filePath, const QString &name, const QPixmap &pixmap, qreal realSize, qreal pxPerFoot)
        : m_realSize(realSize), m_pxPerFoot(pxPerFoot), m_filePath(filePath), originalPixmap(pixmap)
{
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsScenePositionChanges);

    qreal sizePx = realSize * pxPerFoot;

    pixmapItem = new QGraphicsPixmapItem(pixmap.scaled(sizePx, sizePx, Qt::KeepAspectRatio, Qt::SmoothTransformation), this);
    pixmapItem->setOffset(-pixmapItem->boundingRect().width()/2, -pixmapItem->boundingRect().height()/2);

    labelItem = new FixedSizeTextItem(name, this);
    labelItem->setDefaultTextColor(Qt::white);
    labelItem->setOutlineColor(Qt::black);
    labelItem->setOutlineWidth(2.0);
    labelItem->setFont(QFont("Arial", 12));
    labelItem->setPos(0, pixmapItem->boundingRect().height() / 2.0 + 4);
    setTitleDisplayMode(m_mode);
}

void TokenItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QMenu menu;
    QAction* tracker = menu.addAction(tr("Add to tracker"));
    QAction* sheet   = menu.addAction(tr("Open charsheet"));
    QAction* size    = menu.addAction(tr("Change Creature Size"));
    QAction* del     = menu.addAction(tr("Remove from map"));

    QAction* selected = menu.exec(event->screenPos());
    if (selected == tracker) emit addToTracker(m_filePath);
    else if (selected == sheet) emit openCharSheet(m_filePath);
    else if (selected == size) {
        m_realSize = QInputDialog::getInt(nullptr, "Creature size", "Set creature size in feet", m_realSize, 1, 100);
        updateSize();
    }
    else if (selected == del){
        scene()->removeItem(this);
        delete this;
    }
}

void TokenItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsObject::mouseReleaseEvent(event);

    if (scene()) {
        auto* mapScene = qobject_cast<MapScene*>(scene());
        if (mapScene && mapScene->gridEnabled())
            setPos(mapScene->snapToGrid(this->pos(), m_realSize));
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

/**
 *
 * @param step Grid step in feet
 */
void TokenItem::setGridStep(int step) {
    m_gridStep = step;
}

void TokenItem::setTitleDisplayMode(int mode) {
    m_mode = mode;

    switch (m_mode) {
        case TokenTitleDisplayMode::noTitle:
            if (labelItem)
                labelItem->setVisible(false);
            setToolTip(QString());
            break;
        case TokenTitleDisplayMode::tooltip:
            if (labelItem)
                labelItem->setVisible(false);
            setToolTip(labelItem ? labelItem->toPlainText() : QString());
            break;
        default:
            if (labelItem)
                labelItem->setVisible(true);
            setToolTip(QString());
            break;
    }
}

QString TokenItem::stringMode(int mode) {
    switch (mode) {
        case TokenTitleDisplayMode::noTitle:
            return tr("No title");
        case TokenTitleDisplayMode::tooltip:
            return tr("On hover");
        case TokenTitleDisplayMode::always:
            return tr("Always");
        default:
            return tr("N/A");
    }
}

QVariant TokenItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) {
    if (change == ItemPositionHasChanged && scene()){
        QRectF r = mapToScene(boundingRect()).boundingRect();
        scene()->update(r.adjusted(-40, -40, 40, 40));
    }
    return QGraphicsItem::itemChange(change, value);
}

void TokenItem::setFontSize(int size) {
    if (labelItem)
        labelItem->setFont(QFont("Arial", size));
}

void TokenItem::setRealSize(qreal size) {
    m_realSize = size;
    scene()->update();
}

void TokenItem::updateSize() {
    qreal sizePx = m_realSize * m_pxPerFoot;
    QPixmap scaled = originalPixmap.scaled(sizePx, sizePx, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixmapItem->setPixmap(scaled);
    pixmapItem->setOffset(-pixmapItem->boundingRect().width()/2, -pixmapItem->boundingRect().height()/2);
    labelItem->setPos(0, pixmapItem->boundingRect().height() / 2.0 + 4);
    update();
    scene()->update();
}

void TokenItem::updateScaleFactor(qreal footPerPx) {
    m_pxPerFoot = 1/footPerPx;
    updateSize();
}
