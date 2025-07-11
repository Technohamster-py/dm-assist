#include "initiativedelegates.h"


#include <QStyleOptionProgressBar>
#include <QAbstractItemView>
#include <QApplication>
#include <QPainter>
#include <QHelpEvent>
#include <QToolTip>
#include "statuseditdialog.h"

#include <QDebug>


HpProgressBarDelegate::HpProgressBarDelegate(DisplayMode mode, QObject *parent)
        : QStyledItemDelegate(parent), displayMode(mode) {}

void HpProgressBarDelegate::setDisplayMode(DisplayMode mode) {
    displayMode = mode;
}

HpProgressBarDelegate::DisplayMode HpProgressBarDelegate::getDisplayMode() const {
    return displayMode;
}

/**
 * @brief Renders custom visual representation for an item in a view based on the specified display mode.
 *
 * This method provides a custom implementation for painting an item in a view, such as a table or list.
 * The item's appearance is determined by the `displayMode` field, which dictates the type of visual
 * representation to render.
 *
 * @param painter Pointer to the QPainter object used to render the painting.
 * @param option Provides style options and geometry for the item being painted.
 * @param index The model index of the item being painted, providing access to data and roles.
 *
 * The painting behavior is as follows:
 * - If `displayMode` is `Numeric`, the base `QStyledItemDelegate::paint` is used to render the item.
 * - If `displayMode` is `StatusText`, a status string (e.g., "Dead", "Good", "Bad") is calculated
 *   using `calculateHpStatus` and displayed as the item's content.
 * - If `displayMode` is `ProgressBar`, a progress bar is rendered in the item's bounds, showing
 *   the current value and maximum value as specified by the model data associated with `Qt::DisplayRole`
 *   and `Qt::UserRole`, respectively.
 */
void HpProgressBarDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const {
    int value = index.data(Qt::DisplayRole).toInt();
    int max = index.data(Qt::UserRole).toInt();
    if (max <= 0) max = 100;

    switch (displayMode) {
        case Numeric: {
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }
        case StatusText: {
            QString text = calculateHpStatus(value, max);

            QStyleOptionViewItem opt = option;
            initStyleOption(&opt, index);
            opt.text = text;
            QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);
            return;
        }
        case ProgressBar: {
            QStyleOptionProgressBar progressBarOption;
            progressBarOption.rect = option.rect;
            progressBarOption.direction = QApplication::layoutDirection();
            progressBarOption.minimum = 0;
            progressBarOption.maximum = max;
            progressBarOption.progress = value;
            progressBarOption.text = QString("%1/%2").arg(value).arg(max);
            progressBarOption.textVisible = true;
            progressBarOption.textAlignment = Qt::AlignCenter;
            progressBarOption.state = QStyle::StateFlag::State_Horizontal;
            QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
            return;
        }
    }
}

/**
 * @brief Evaluates the health point (HP) status based on current and maximum HP.
 *
 * This function determines the status of an entity's health based on the ratio of current HP
 * to maximum HP. If the HP is less than or equal to 0, the status is "Dead". If the ratio of HP
 * to max HP is greater than 0.5, the status is considered "Good". Otherwise, the status is "Bad".
 *
 * @param hp The current health points of the entity.
 * @param maxHp The maximum possible health points of the entity.
 * @return A QString indicating the health status: "Dead", "Good", or "Bad".
 */
static QString calculateHpStatus(int hp, int maxHp)
{
    double ratio = (double)hp / maxHp;
    if (hp <= 0)
        return "Dead";
    else if (ratio > 0.5)
        return "Good";
    else
        return "Bad";
}

void StatusDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->save();

    QVariant data = index.data(Qt::UserRole + 1);
    if (data.canConvert<QList<Status>>()) {
        QList<Status> statuses = data.value<QList<Status>>();

        int x = option.rect.x();
        int y = option.rect.y();
        int iconSize = option.rect.height();

        for (const auto &status : statuses) {
            QPixmap icon(status.iconPath);
            QRect iconRect(x, y, iconSize, iconSize);
            painter->drawPixmap(iconRect, icon);
            x += iconSize + 2; // padding
        }
    }

    painter->restore();
}

bool StatusDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option,
                               const QModelIndex &index) {
    if (!event || !view || !index.isValid())
        return false;

    if (event->type() == QEvent::ToolTip) {
        QVariant statusData = index.data(Qt::UserRole + 1);
        if (statusData.canConvert<QList<Status>>()) {
            QString tooltip;
            const QList<Status> statuses = statusData.value<QList<Status>>();
            for (const auto &status : statuses) {
                tooltip += QString("%1 (%2 раундов)").arg(status.title).arg(status.remainingRounds) + "\n";
            }

            if (!tooltip.isEmpty()) {
                QToolTip::showText(event->globalPos(), tooltip.trimmed(), view->viewport());
                return true;
            }
        }
    }

    return QStyledItemDelegate::helpEvent(event, view, option, index);
}

bool StatusDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                                 const QModelIndex &index) {
    if (event->type() == QEvent::MouseButtonDblClick && index.isValid()) {

        auto statuses = index.data(Qt::UserRole + 1).value<QList<Status>>();
        auto *dialog = new StatusEditDialog(statuses);


        QPoint globalPos = option.widget->mapToGlobal(option.rect.bottomLeft());
        QSize dialogSize = dialog->sizeHint();
        QScreen *screen = option.widget->screen();
        QRect screenRect = screen->availableGeometry();

        if (globalPos.x() + dialogSize.width() > screenRect.right())
            globalPos.setX(screenRect.right() - dialogSize.width() - 50);

        if (globalPos.y() + dialogSize.height() > screenRect.bottom())
            globalPos.setY(screenRect.bottom() - dialogSize.height() - 50);

        globalPos.setX(std::max(screenRect.left(), globalPos.x()));
        globalPos.setY(std::max(screenRect.top(), globalPos.y()));


        dialog->move(globalPos);

        connect(dialog, &QDialog::finished, this, [=](int result) {
            QVariant newStatusList = QVariant::fromValue(dialog->statuses());
            model->setData(index, newStatusList);
        });
        dialog->show();
        return true;
    }
    return false;
}
