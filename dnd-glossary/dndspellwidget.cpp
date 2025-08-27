#include "dndspellwidget.h"
#include "ui_dndspellwidget.h"


dndSpellWidget::dndSpellWidget(QWidget *parent) :
        QWidget(parent), ui(new Ui::dndSpellWidget) {
    ui->setupUi(this);
}

dndSpellWidget::~dndSpellWidget() {
    delete ui;
}
