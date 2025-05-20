#include "HoverWidget.h"

HoverWidget::HoverWidget(const QString &text, QWidget *parent)
        : QWidget(parent)
{
    label = new QLabel(text);
    editButton = new QPushButton();
    editButton->setIcon(QIcon(":/edit.svg"));
    editButton->setMaximumWidth(20);
    activateButton = new QPushButton();
    activateButton->setIcon(QIcon(":/upload.svg"));
    activateButton->setMaximumWidth(20);

    editButton->setVisible(false);
    activateButton->setVisible(false);

    auto layout = new QHBoxLayout(this);
    layout->addWidget(label);
    layout->addWidget(editButton);
    layout->addWidget(activateButton);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    setAttribute(Qt::WA_Hover);
    installEventFilter(this);

    connect(editButton, &QPushButton::clicked, this, &HoverWidget::editClicked);
    connect(activateButton, &QPushButton::clicked, this, &HoverWidget::activateClicked);
}

bool HoverWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        editButton->setVisible(true);
        activateButton->setVisible(true);
    } else if (event->type() == QEvent::Leave) {
        editButton->setVisible(false);
        activateButton->setVisible(false);
    }
    return QWidget::eventFilter(obj, event);
}
