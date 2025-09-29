#pragma once

#include <QTreeWidget>
#include <QDir>
#include "hoverwidget.h"

class CampaignTreeWidget : public QTreeWidget{
    Q_OBJECT
public:
    explicit CampaignTreeWidget(QWidget* parent = nullptr);

    bool setRootDir(const QString &rootPath);
    QString root() const {return m_rootPath;};
    QString campaignName() const {return m_campaignName;};

    static NodeType determieNodeType(const QString& path);

signals:
    void characterOpenRequested(const QString& path);
    void characterAddRequested(const QString& path);

    void encounterAddRequested(const QString& path);
    void encounterReplaceRequested(const QString& path);

    void mapOpenRequested(const QString& path);

    void beastOpenRequested(const QString& path);
    void beastAddRequested(const QString& path);

    void campaignLoaded(const QString &name);

public slots:
    void showContextMenu(const QPoint &pos);

protected:
    void startDrag(Qt::DropActions) override;

private:
    void populateTree(const QString &path, QTreeWidgetItem *parentItem);
    bool ignore(const QFileInfo& info);

    QString m_rootPath;
    QString m_campaignName;

    static bool isValidCampaignRoot(const QString &rootPath);
    static QString loadCampaignName(const QString &rootPath);
};