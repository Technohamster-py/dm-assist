#include "hoverwidget.h"
#include <themediconmanager.h>

/**
 * @brief Constructor for the HoverWidget class.
 *
 * This constructor initializes the HoverWidget with a given text, node type, and optional parent widget.
 * It sets up the internal UI components such as a label and buttons, configures the layout,
 * and installs the necessary event filters and connections.
 *
 * @param text The text to display on the QLabel within the widget.
 * @param type The NodeType which defines the behavior and visibility of the buttons.
 * @param parent The parent QWidget for this HoverWidget. Defaults to nullptr.
 */
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

/**
 * Filters events to handle hover animations for associated actions.
 *
 * This function is an overridden implementation of the `eventFilter` method
 * and is used to detect `QEvent::Enter` and `QEvent::Leave` events on the
 * widget. It responds to these events by modifying the visibility of the
 * `action1` and `action2` buttons based on their icon state. If the event
 * is of type `QEvent::Enter`, it makes the buttons visible if their icons
 * are not null. Conversely, on a `QEvent::Leave` event, the buttons are
 * set to be invisible.
 *
 * @param obj The object that the event is associated with.
 * @param event The event to be filtered and processed.
 * @return Returns true if the event is handled by the filter; otherwise,
 *         it falls back to the base class implementation and returns the result.
 */
bool HoverWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        if (!action1->icon().isNull())
            action1->setVisible(true);
        if (!action2->icon().isNull())
            action2->setVisible(true);
    } else if (event->type() == QEvent::Leave) {
        action1->setVisible(false);
        action2->setVisible(false);
    }
    return QWidget::eventFilter(obj, event);
}

/**
 * @brief Configures the appearance and behavior of action buttons based on the specified node type.
 *
 * This function adjusts the properties of `action1` and `action2` buttons, such as their maximum width, icons, visibility,
 * and tooltips, according to the provided `NodeType`.
 *
 * - For `NodeType::Character`, both buttons are visible. `action1` is configured with an "Edit" icon and tooltip, and
 *   `action2` with an "Add to current encounter" icon and tooltip.
 * - For `NodeType::Encounter`, both buttons are visible. `action1` is configured with an "Add to current encounter" icon
 *   and tooltip, and `action2` with a "Replace current encounter" icon and tooltip.
 * - For `NodeType::Map`, `action1` is configured with an "Edit" icon and tooltip while `action2` is hidden.
 * - For any other `NodeType`, both `action1` and `action2` are hidden.
 *
 * @param type The node type based on which the action buttons are configured.
 */
void HoverWidget::setupButtons(NodeType type) {
    action1->setMaximumWidth(20);
    action2->setMaximumWidth(20);
    switch (type) {
        case NodeType::Character:
            ThemedIconManager::instance().addIconTarget<QAbstractButton>(":/edit.svg", action1, &QAbstractButton::setIcon);
            action1->setToolTip(tr("Edit"));
            ThemedIconManager::instance().addIconTarget<QAbstractButton>(":/add.svg", action2, &QAbstractButton::setIcon);
            action2->setToolTip(tr("Add to current encounter"));
            break;
        case NodeType::Encounter:
            ThemedIconManager::instance().addIconTarget<QAbstractButton>(":/add.svg", action1, &QAbstractButton::setIcon);
            action1->setToolTip(tr("Add to current encounter"));
            ThemedIconManager::instance().addIconTarget<QAbstractButton>(":/upload.svg", action2, &QAbstractButton::setIcon);
            action2->setToolTip(tr("Replace current encounter"));
            break;
        case NodeType::Map:
            ThemedIconManager::instance().addIconTarget<QAbstractButton>(":/edit.svg", action1, &QAbstractButton::setIcon);
            action1->setToolTip(tr("Edit"));
            action2->hide();
            break;
        case NodeType::Beast:
            ThemedIconManager::instance().addIconTarget<QAbstractButton>(":/edit.svg", action1, &QAbstractButton::setIcon);
            action1->setToolTip(tr("View"));
            ThemedIconManager::instance().addIconTarget<QAbstractButton>(":/add.svg", action2, &QAbstractButton::setIcon);
            action2->setToolTip(tr("Add to current encounter"));
            break;
        case NodeType::Spell:
            ThemedIconManager::instance().addIconTarget<QAbstractButton>(":/edit.svg", action1, &QAbstractButton::setIcon);
            action1->setToolTip(tr("Edit"));
            ThemedIconManager::instance().addIconTarget<QAbstractButton>(":/add.svg", action2, &QAbstractButton::setIcon);
            action2->setToolTip(tr("Set as painter"));
            break;
        default:
            action1->hide();
            action2->hide();
            break;
    }
}
