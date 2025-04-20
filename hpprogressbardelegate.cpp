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


QSize HpProgressBarDelegate::sizeHint(const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const {
    return QSize(option.rect.width(), 20);
}

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