#include "hoverwidget.h"

HoverWidget::HoverWidget(const QString &text, NodeType type, QWidget *parent)
        : QWidget(parent)
{
    label = new QLabel(text);
    m_type = type;
    action1 = new QPushButton();
    action2 = new QPushButton();

    setupButtons(type);

    action1->setVisible(false);
    action2->setVisible(false);

    auto layout = new QHBoxLayout(this);
    layout->addWidget(label);
    layout->addWidget(action1);
    layout->addWidget(action2);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    setAttribute(Qt::WA_Hover);
    installEventFilter(this);

    connect(action1, &QPushButton::clicked, this, &HoverWidget::action1Clicked);
    connect(action2, &QPushButton::clicked, this, &HoverWidget::action2Clicked);
}

bool HoverWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        action1->setVisible(true);
        action2->setVisible(true);
    } else if (event->type() == QEvent::Leave) {
        action1->setVisible(false);
        action2->setVisible(false);
    }
    return QWidget::eventFilter(obj, event);
}

void HoverWidget::setupButtons(NodeType type) {
    action1->setMaximumWidth(20);
    action2->setMaximumWidth(20);
    switch (type) {
        case NodeType::Character:
            action1->setIcon(QIcon(":/edit.svg"));
            action2->setIcon(QIcon(":/add.svg"));
            break;
        case NodeType::Encounter:
            action1->setIcon(QIcon(":/add.svg"));
            action2->setIcon(QIcon(":/upload.svg"));
            break;
        case NodeType::Map:
            action1->setIcon(QIcon(":/edit.svg"));
            action2->hide();
        default:
            action1->hide();
            action2->hide();
            break;
    }
}
