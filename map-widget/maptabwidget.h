#ifndef DM_ASSIST_MAPTABWIDGET_H
#define DM_ASSIST_MAPTABWIDGET_H

#include <QApplication>
#include <QTabWidget>
#include <QTabBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QLabel>

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
    }

signals:
    void newTabRequested();

private slots:
    void addNewTab() {
        emit newTabRequested();
    }
};

#endif //DM_ASSIST_MAPTABWIDGET_H
