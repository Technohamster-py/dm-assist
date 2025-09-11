#ifndef DM_ASSIST_TEXTCLICKABLECHECKBOX_H
#define DM_ASSIST_TEXTCLICKABLECHECKBOX_H

#include <QCheckBox>
#include <QLabel>
#include <QMouseEvent>

/**
 * @class TextClickableCheckBox
 * @brief A custom implementation of a checkbox with clickable text.
 *
 * This class extends the functionality of a standard checkbox by allowing
 * the accompanying text label to be clickable as well. Clicking the label
 * do not toggle the state of the checkbox.
 */
class TextClickableCheckBox : public QCheckBox{
    Q_OBJECT
public:
    using QCheckBox::QCheckBox;

signals:
    void textClicked();
protected:
    void mousePressEvent(QMouseEvent* event) override;
};


/**
 * @class ClickableLabel
 * @brief A customizable label widget that supports click interactions.
 *
 * ClickableLabel extends a standard label to include the capability of detecting
 * and responding to user click events. This class can be used in applications
 * where interactive text or graphics are required without using a button widget.
 *
 * Features:
 * - Detects and handles mouse click events on the label.
 * - Can be used to trigger custom actions or propagate events upon interaction.
 * - Retains all the standard functionality of a label widget (e.g., text display, styling).
 *
 * This class is intended to provide enhanced interactivity for GUI designs while
 * maintaining seamless integration with commonly used graphical frameworks.
 */
class ClickableLabel : public QLabel{
    Q_OBJECT
public:
    using QLabel::QLabel;
signals:
    void clicked();
protected:
    void mousePressEvent(QMouseEvent *event) override;
};


#endif //DM_ASSIST_TEXTCLICKABLECHECKBOX_H
