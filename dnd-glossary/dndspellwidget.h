//
// Created by akhomchenko on 08.04.24.
//

#ifndef DM_ASSIST_DNDSPELLWIDGET_H
#define DM_ASSIST_DNDSPELLWIDGET_H

#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class dndSpellWidget; }
QT_END_NAMESPACE

class dndSpellWidget : public QWidget {
Q_OBJECT

public:
    explicit dndSpellWidget(QWidget *parent = nullptr);

    ~dndSpellWidget() override;

private:
    Ui::dndSpellWidget *ui;
};


#endif //DM_ASSIST_DNDSPELLWIDGET_H
