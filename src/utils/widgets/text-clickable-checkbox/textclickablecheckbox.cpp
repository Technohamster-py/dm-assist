#include <QStyleOptionButton>
#include "textclickablecheckbox.h"

/**
 * @brief Handles the mouse press event to differentiate between text and checkbox clicks.
 *
 * This method overrides the default `mousePressEvent` to determine if the mouse click
 * occurred on the text label portion of the checkbox or on the checkbox indicator itself.
 * If the text is clicked, the `textClicked` signal is emitted; otherwise, the default
 * checkbox behavior is executed.
 *
 * @param event Pointer to the QMouseEvent representing the mouse press event.
 */
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
