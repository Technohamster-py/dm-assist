#include "maptabwidget.h"
#include <QMimeData>
#include <QFileInfo>

TabWidget::TabWidget(QWidget *parent) : QTabWidget(parent) {
    setTabsClosable(true);
    setTabBarAutoHide(false);
    setAcceptDrops(true);

    auto *plusButton = new QToolButton(this);
    ThemedIconManager::instance().addIconTarget<QAbstractButton>(":/add.svg", plusButton, &QAbstractButton::setIcon);
    plusButton->setAutoRaise(true);
    plusButton->setToolTip(tr("Open new map"));

    connect(plusButton, &QToolButton::clicked, this, &TabWidget::addNewTab);

    auto *cornerWidget = new QWidget(this);
    auto *layout = new QHBoxLayout(cornerWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addStretch();
    layout->addWidget(plusButton);

    setCornerWidget(cornerWidget, Qt::TopRightCorner);

    tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tabBar(), &QTabBar::customContextMenuRequested, this, &TabWidget::showContextMenu);
}

void TabWidget::showContextMenu(const QPoint &pos) {
    int index = tabBar()->tabAt(pos);
    if (index < 0) return;

    QMenu menu;

    QAction *shareAction = menu.addAction(tr("Share"));
    connect(shareAction, &QAction::triggered, this, [=]() {emit share(index);});

    QAction *saveAction = menu.addAction(tr("Save"));
    connect(saveAction, &QAction::triggered, this, [=]() {emit save(index);});

    menu.exec(tabBar()->mapToGlobal(pos));
}

void TabWidget::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void TabWidget::dropEvent(QDropEvent *event) {
    QStringList files;
    for (const QUrl &url : event->mimeData()->urls()) {
        QString path = url.toLocalFile();
        QFileInfo info(path);
        if (info.exists() && info.isFile() && (info.suffix().toLower() == "dam" || info.suffix().toLower() == "png" || info.suffix().toLower() == "jpg" || info.suffix().toLower() == "bmp")) {
            files.append(path);
        }
    }

    if (!files.isEmpty())
    {
        for (const QString &file : files)
            emit dropAccepted(file);
    }
}
