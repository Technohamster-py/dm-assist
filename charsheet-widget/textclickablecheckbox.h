#ifndef DM_ASSIST_TEXTCLICKABLECHECKBOX_H
#define DM_ASSIST_TEXTCLICKABLECHECKBOX_H

#include <QCheckBox>
#include <QLabel>
#include <QMouseEvent>

class TextClickableCheckBox : public QCheckBox{
    Q_OBJECT
public:
    using QCheckBox::QCheckBox;

signals:
    void textClicked();
protected:
    void mousePressEvent(QMouseEvent* event) override;
};


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
