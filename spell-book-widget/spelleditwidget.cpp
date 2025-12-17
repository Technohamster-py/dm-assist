// You may need to build the project (run Qt uic code generator) to get "ui_SpellEditWidget.h" resolved

#include "spelleditwidget.h"
#include "ui_spelleditwidget.h"


SpellEditWidget::SpellEditWidget(QWidget *parent) :
        QWidget(parent), ui(new Ui::SpellEditWidget) {
    ui->setupUi(this);
}

SpellEditWidget::~SpellEditWidget() {
    delete ui;
}
