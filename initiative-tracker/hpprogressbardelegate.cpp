#include "hpprogressbardelegate.h"


#include <QStyleOptionProgressBar>
#include <QApplication>
#include <QPainter>


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
            progressBarOption.minimum = 0;
            progressBarOption.maximum = max;
            progressBarOption.progress = value;
            progressBarOption.text = QString("%1/%2").arg(value).arg(max);
            progressBarOption.textVisible = false;
            progressBarOption.textAlignment = Qt::AlignCenter;
            progressBarOption.state = QStyle::State_Enabled;
            QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
            return;
        }
    }
}


/**
 * @brief Provides size hint for the progress bar delegate.
 *
 * This method calculates and returns the size hint for rendering
 * a custom progress bar delegate within a view. The returned size
 * specifies the recommended width and height of the delegate.
 *
 * @param option The style options for the item, containing details
 *               about the item's geometry and state.
 * @param index The model index that identifies the item in the model.
 * @return QSize The recommended size for the delegate. The width
 *               matches the width of the item's rectangle, and the
 *               height is set to a fixed value of 20 pixels.
 */
QSize HpProgressBarDelegate::sizeHint(const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const {
    return QSize(option.rect.width(), 20);
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