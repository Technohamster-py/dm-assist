//
// Created by akhomchenko on 02.05.24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_qDndInitiativeEntityEditWidget.h" resolved

#include "qdndinitiativeentityeditwidget.h"
#include "ui_qdndinitiativeentityeditwidget.h"


qDndInitiativeEntityEditWidget::qDndInitiativeEntityEditWidget(EncounterEntity *entity, QWidget *parent) :
        QWidget(parent), ui(new Ui::qDndInitiativeEntityEditWidget) {
    ui->setupUi(this);

    ui->nameLabel->setText(entity->getTitle());
    ui->acLabel->setText(QString::number(entity->getAC()));
    ui->hpSpinBox->setValue(entity->getHP());
    ui->hpSpinBox->setMaximum(entity->getMaxHp());
}

qDndInitiativeEntityEditWidget::~qDndInitiativeEntityEditWidget() {
    delete ui;
}
