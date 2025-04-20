#ifndef DM_ASSIST_HPPROGRESSBARDELEGATE_H
#define DM_ASSIST_HPPROGRESSBARDELEGATE_H

#pragma once
#include <QStyledItemDelegate>

static QString calculateHpStatus(int hp, int maxHp);

/**
 * @brief Делегат для отрисовки HP в виде прогресс-бара
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
