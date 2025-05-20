#pragma once

#include <QTreeWidget>
#include <QDir>

class CampaignTreeWidget : public QTreeWidget{
    Q_OBJECT
public:
    explicit CampaignTreeWidget(QWidget* parent = nullptr);

    void setRootDir(const QString &rootPath);

signals:
    void editRequested(const QString& path);
    void activateRequested(const QString& path);

private:
    void populateTree(const QString &path, QTreeWidgetItem *parentItem);
};