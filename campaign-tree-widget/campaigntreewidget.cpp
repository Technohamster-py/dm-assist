#include "campaigntreewidget.h"
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

CampaignTreeWidget::CampaignTreeWidget(QWidget *parent) : QTreeWidget(parent)
{
    setColumnCount(1);
    setHeaderLabel(tr("Campaign Structure"));
    m_rootPath = QString();
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

bool CampaignTreeWidget::setRootDir(const QString &rootPath)
{
    if (!isValidCampaignRoot(rootPath)) {
        QMessageBox::warning(this, "Error", tr("The selected folder does not contain campaign.json is not a campaign"));
        return false;
    }

    clear();
    m_rootPath = QDir(rootPath).absolutePath();
    m_campaignName = loadCampaignName(m_rootPath);

    auto *rootItem = new QTreeWidgetItem(this);
    auto *widget = new HoverWidget(m_campaignName, NodeType::Unknown, this);
    setItemWidget(rootItem, 0, widget);

    populateTree(rootPath, rootItem);
    expandAll();

    emit campaignLoaded(m_campaignName);
    return true;
}

bool CampaignTreeWidget::isValidCampaignRoot(const QString &rootPath) {
    QFile markerFile(rootPath + "/campaign.json");
    return markerFile.exists();
}

QString CampaignTreeWidget::loadCampaignName(const QString &rootPath) {
    QFile file(rootPath + "/campaign.json");
    if (!file.open(QIODevice::ReadOnly))
        return "Unnamed Campaign";

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject())
        return "Unnamed Campaign";

    QJsonObject obj = doc.object();
    return obj.value("name").toString("Unnamed Campaign");
}
