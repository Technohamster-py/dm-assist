#include "qdndcharacterwidget.h"
#include "ui_qdndcharacterwidget.h"


QDndCharacterWidget::QDndCharacterWidget() :
        QWidget(parent), ui(new Ui::QDndCharacterWidget) {
    ui->setupUi(this);
}

QDndCharacterWidget::~QDndCharacterWidget() {
    delete ui;
}
