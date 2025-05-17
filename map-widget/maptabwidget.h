#ifndef DM_ASSIST_MAPTABWIDGET_H
#define DM_ASSIST_MAPTABWIDGET_H

#include <QApplication>
#include <QTabWidget>
#include <QTabBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QMenu>

class TabWidget : public QTabWidget {
Q_OBJECT

public:
    TabWidget(QWidget *parent = nullptr) : QTabWidget(parent) {
        setTabsClosable(true);
        setTabBarAutoHide(false);

        QToolButton *plusButton = new QToolButton(this);
        plusButton->setIcon(QIcon(":/add.svg"));
        plusButton->setAutoRaise(true);
        plusButton->setToolTip(tr("Open new map"));

        connect(plusButton, &QToolButton::clicked, this, &TabWidget::addNewTab);

        QWidget *cornerWidget = new QWidget(this);
        QHBoxLayout *layout = new QHBoxLayout(cornerWidget);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addStretch();
        layout->addWidget(plusButton);

        setCornerWidget(cornerWidget, Qt::TopRightCorner);

        tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(tabBar(), &QTabBar::customContextMenuRequested, this, &TabWidget::showContextMenu);
    }

signals:
    void newTabRequested();
    void share(int tab);
    void save(int tab);

private slots:
    void addNewTab() {
        emit newTabRequested();
    }

    void showContextMenu(const QPoint &pos){
        int index = tabBar()->tabAt(pos);
        if (index < 0) return;

        QMenu menu;

        QAction *shareAction = menu.addAction(tr("Share"));
        connect(shareAction, &QAction::triggered, this, [=]() {emit share(index);});

        QAction *saveAction = menu.addAction(tr("Save"));
        connect(saveAction, &QAction::triggered, this, [=]() {emit save(index);});

        menu.exec(tabBar()->mapToGlobal(pos));
    }
};

#endif //DM_ASSIST_MAPTABWIDGET_H
