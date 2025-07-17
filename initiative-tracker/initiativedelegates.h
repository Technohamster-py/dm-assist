#ifndef DM_ASSIST_INITIATIVEDELEGATES_H
#define DM_ASSIST_INITIATIVEDELEGATES_H

#pragma once
#include <QStyledItemDelegate>
#include "initiativestructures.h"

static QString calculateHpStatus(int hp, int maxHp);


/**
 * @class HpProgressBarDelegate
 * @brief A custom delegate for rendering numeric values, status text, or progress bars in item views.
 *
 * HpProgressBarDelegate inherits from QStyledItemDelegate and provides functionality
 * to display health point (HP) or progress-related data in three different visual modes:
 * Numeric, StatusText, and ProgressBar.
 */
class HpProgressBarDelegate : public QStyledItemDelegate {
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

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    DisplayMode displayMode;
};


class StatusDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};


#endif //DM_ASSIST_INITIATIVEDELEGATES_H
