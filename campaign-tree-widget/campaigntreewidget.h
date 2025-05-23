#pragma once

#include <QTreeWidget>
#include <QDir>
#include "hoverwidget.h"

class CampaignTreeWidget : public QTreeWidget{
    Q_OBJECT
public:
    explicit CampaignTreeWidget(QWidget* parent = nullptr);

    void setRootDir(const QString &rootPath);

signals:
    void characterOpenRequested(const QString& path);
    void characterAddRequested(const QString& path);

    void encounterAddRequested(const QString& path);
    void encounterReplaceRequested(const QString& path);

    void mapOpenRequested(const QString& path);

private:
    void populateTree(const QString &path, QTreeWidgetItem *parentItem);
    NodeType determieNodeType(const QString& path);
    bool ignore(const QFileInfo& info);

    QString m_rootPath;
};