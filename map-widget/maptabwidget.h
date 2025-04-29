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
        QToolButton *plusButton = new QToolButton(this);
        plusButton->setText("+");
        plusButton->setAutoRaise(true); // без рамок, как иконка
        plusButton->setToolTip("Создать новую вкладку");

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
