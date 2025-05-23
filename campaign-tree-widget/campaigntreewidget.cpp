#include "campaigntreewidget.h"
#include <QFileInfo>

CampaignTreeWidget::CampaignTreeWidget(QWidget *parent) : QTreeWidget(parent)
{
    setColumnCount(1);
    setHeaderLabel(tr("Campaign Structure"));
}

NodeType CampaignTreeWidget::determieNodeType(const QString &path)
{
    if (path.contains("/Characters"))
        return NodeType::Character;
    if (path.contains("/Encounters"))
        return NodeType::Encounter;
    if (path.contains("/Maps"))
        return NodeType::Map;
    if (path.contains("/Music"))
        return NodeType::Music;
    return NodeType::Unknown;
}


bool CampaignTreeWidget::ignore(const QFileInfo &info)
{
    QString relativePath = QDir(m_rootPath).relativeFilePath(info.absoluteFilePath());
    QString normalized = QDir::cleanPath(relativePath).toLower();

    if (normalized.startsWith("music/") || normalized == "music")
        return true;
    if (normalized == "playerConfig.xml" || normalized == "root")
        return true;
    return false;
}

void CampaignTreeWidget::populateTree(const QString &path, QTreeWidgetItem *parentItem)
{
    QDir dir(path);
    QFileInfoList entries = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);

    for (const QFileInfo &entry : entries) {
        if (ignore(entry))
            continue;

        NodeType type = determieNodeType(entry.absoluteFilePath());
        auto *item = new QTreeWidgetItem();
        parentItem->addChild(item);

        auto *widget = new HoverWidget(entry.fileName(), type);
        setItemWidget(item, 0, widget);

        QString fullPath = entry.absoluteFilePath();

        switch (type) {
            case NodeType::Character:
                connect(widget, &HoverWidget::action1Clicked, this, [=](){ emit characterOpenRequested(fullPath); });
                connect(widget, &HoverWidget::action2Clicked, this, [=](){ emit characterAddRequested(fullPath); });
                break;
            case NodeType::Encounter:
                connect(widget, &HoverWidget::action1Clicked, this, [=](){ emit encounterAddRequested(fullPath); });
                connect(widget, &HoverWidget::action2Clicked, this, [=](){ emit encounterReplaceRequested(fullPath); });
                break;
            case NodeType::Map:
                connect(widget, &HoverWidget::action1Clicked, this, [=](){ emit mapOpenRequested(fullPath); });
                break;
            default:
                break;
        }

        if (entry.isDir()) {
            populateTree(entry.absoluteFilePath(), item);
        }
    }
}

void CampaignTreeWidget::setRootDir(const QString &rootPath)
{
    clear();
    m_rootPath = QDir(rootPath).absolutePath();
    auto *rootItem = new QTreeWidgetItem(this);
    auto *widget = new HoverWidget(QFileInfo(rootPath).fileName(), NodeType::Unknown);
    setItemWidget(rootItem, 0, widget);
    populateTree(rootPath, rootItem);
    expandAll();
}