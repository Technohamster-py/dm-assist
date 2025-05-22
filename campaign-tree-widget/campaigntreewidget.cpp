#include "campaigntreewidget.h"
#include "hoverwidget.h"
#include <QFileInfo>
#include <QFile>

#include <QDebug>

CampaignTreeWidget::CampaignTreeWidget(QWidget *parent) : QTreeWidget(parent) {
    setColumnCount(1);
    setHeaderLabel(tr("Campaign Structure"));

    setStyleSheet("QTreeWidget { background: transparent; }");

    setAttribute(Qt::WA_TranslucentBackground);
    setFrameStyle(QFrame::NoFrame);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void CampaignTreeWidget::setRootDir(const QString &rootPath) {
    clear();

    auto *rootItem = new QTreeWidgetItem(this);
    auto *rootWidget = new HoverWidget(QFileInfo(rootPath).fileName());
    setItemWidget(rootItem, 0, rootWidget);

    populateTree(rootPath, rootItem);
    expandAll();
}

void CampaignTreeWidget::populateTree(const QString &path, QTreeWidgetItem *parentItem) {
    QDir dir(path);
    QFileInfoList entries = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);

    for (const QFileInfo &entry : entries) {
        if (entry.isDir()) {
            QString testPath = entry.absoluteFilePath() + ".xml";
            if (QFile::exists(testPath)) continue;

            auto *item = new QTreeWidgetItem(parentItem);
            parentItem->addChild(item);
            auto *widget = new QLabel(entry.fileName());
            setItemWidget(item, 0, widget);

            populateTree(entry.absoluteFilePath(), item);
        } else {
            auto *item = new QTreeWidgetItem(parentItem);
            parentItem->addChild(item);
            auto *widget = new HoverWidget(entry.fileName());
            setItemWidget(item, 0, widget);

            connect(widget, &HoverWidget::editClicked, this, [this, entry](){
                emit editRequested(entry.absoluteFilePath());
            });

            connect(widget, &HoverWidget::activateClicked, this, [this, entry](){
                emit activateRequested(entry.absoluteFilePath());
            });
        }
    }
}
