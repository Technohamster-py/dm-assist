//
// Created by akhomchenko on 02.05.24.
//

#ifndef DM_ASSIST_QDNDINITIATIVEENTITYEDITWIDGET_H
#define DM_ASSIST_QDNDINITIATIVEENTITYEDITWIDGET_H

#include <QWidget>
#include "encounter.h"

QT_BEGIN_NAMESPACE
namespace Ui { class qDndInitiativeEntityEditWidget; }
QT_END_NAMESPACE

class qDndInitiativeEntityEditWidget : public QWidget {
Q_OBJECT

public:
    explicit qDndInitiativeEntityEditWidget(EncounterEntity *entity, QWidget *parent = nullptr);

    ~qDndInitiativeEntityEditWidget() override;

private:
    Ui::qDndInitiativeEntityEditWidget *ui;
};


#endif //DM_ASSIST_QDNDINITIATIVEENTITYEDITWIDGET_H
