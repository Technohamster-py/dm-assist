#include "bestiarypage.h"
#include "ui_bestiarypage.h"


BestiaryPage::BestiaryPage(QWidget *parent) :
        QWidget(parent), ui(new Ui::BestiaryPage) {
    ui->setupUi(this);
}

BestiaryPage::~BestiaryPage() {
    delete ui;
}
