#ifndef DM_ASSIST_SPELLEDITWIDGET_H
#define DM_ASSIST_SPELLEDITWIDGET_H

#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class SpellEditWidget; }
QT_END_NAMESPACE

class SpellEditWidget : public QWidget {
Q_OBJECT

public:
    explicit SpellEditWidget(QWidget *parent = nullptr);

    ~SpellEditWidget() override;

private:
    Ui::SpellEditWidget *ui;
};


#endif //DM_ASSIST_SPELLEDITWIDGET_H
