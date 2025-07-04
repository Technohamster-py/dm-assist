#ifndef DM_ASSIST_MAPTABWIDGET_H
#define DM_ASSIST_MAPTABWIDGET_H

#include <QApplication>
#include <QTabWidget>
#include <QTabBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QMenu>

#include <themediconmanager.h>

/**
 * @class TabWidget
 * @brief A custom widget derived from QTabWidget with added functionality such as "new tab"
 *        button and a context menu for tab management.
 *
 * The TabWidget class provides a specialized QTabWidget that includes:
 * - A corner "+" button to add new tabs.
 * - A context menu appearing on right-clicking a tab for performing actions like "share" and "save".
 * - Integration with signals for handling tab-related actions externally.
 */
class TabWidget : public QTabWidget {
Q_OBJECT

public:
    explicit TabWidget(QWidget *parent = nullptr) : QTabWidget(parent) {
        setTabsClosable(true);
        setTabBarAutoHide(false);

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
