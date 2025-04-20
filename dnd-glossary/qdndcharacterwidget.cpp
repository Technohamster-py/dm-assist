//
// Created by arseniy on 02.04.2024.
//

// You may need to build the project (run Qt uic code generator) to get "ui_QDndCharacterWidget.h" resolved

#include "qdndcharacterwidget.h"
#include "ui_qdndcharacterwidget.h"


QDndCharacterWidget::QDndCharacterWidget(DndCharacter character, QWidget *parent) :
        QWidget(parent), ui(new Ui::QDndCharacterWidget) {
    ui->setupUi(this);
}

QDndCharacterWidget::~QDndCharacterWidget() {
    delete ui;
}
