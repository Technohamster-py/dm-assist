#ifndef DM_ASSIST_HPPROGRESSBARDELEGATE_H
#define DM_ASSIST_HPPROGRESSBARDELEGATE_H

#pragma once
#include <QStyledItemDelegate>

static QString calculateHpStatus(int hp, int maxHp);


/**
 * @class HpProgressBarDelegate
 * @brief A custom delegate for rendering numeric values, status text, or progress bars in item views.
 *
 * HpProgressBarDelegate inherits from QStyledItemDelegate and provides functionality
 * to display health point (HP) or progress-related data in three different visual modes:
 * Numeric, StatusText, and ProgressBar.
 */
class HpProgressBarDelegate : public QStyledItemDelegate
{
Q_OBJECT
public:
    enum DisplayMode {
        Numeric,
        StatusText,
        ProgressBar
    };
    explicit HpProgressBarDelegate(DisplayMode mode = ProgressBar, QObject *parent = nullptr);

    void setDisplayMode(DisplayMode mode);
    DisplayMode getDisplayMode() const;

    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

private:
    DisplayMode displayMode;
};


#endif //DM_ASSIST_HPPROGRESSBARDELEGATE_H
