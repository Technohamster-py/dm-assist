//
// Created by akhomchenko on 08.04.24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_dndSpellWidget.h" resolved

#include "dndspellwidget.h"
#include "ui_dndspellwidget.h"


dndSpellWidget::dndSpellWidget(QWidget *parent) :
        QWidget(parent), ui(new Ui::dndSpellWidget) {
    ui->setupUi(this);
}

dndSpellWidget::~dndSpellWidget() {
    delete ui;
}
