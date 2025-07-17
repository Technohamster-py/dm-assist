#include <QStyleOptionButton>
#include "textclickablecheckbox.h"

void TextClickableCheckBox::mousePressEvent(QMouseEvent *event) {
    QStyleOptionButton opt;
    initStyleOption(&opt);
    QRect cbRect = style()->subElementRect(QStyle::SE_CheckBoxIndicator, &opt, this);
    if (!cbRect.contains(event->pos()))
    {
        emit textClicked();
        return;
    }
    QCheckBox::mousePressEvent(event);
}

void ClickableLabel::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
    QLabel::mousePressEvent(event);
}
