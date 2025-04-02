//
// Created by arseniy on 02.04.2025.
//

#include "settingsdialog.h"
#include "ui_SettingsDialog.h"


SettingsDialog::SettingsDialog(QWidget *parent) :
        QDialog(parent), ui(new Ui::SettingsDialog) {
    ui->setupUi(this);

    ui->navTree->expandAll();
    ui->navTree->setColumnHidden(1, true);

    connect(ui->navTree, &QTreeWidget::currentItemChanged, this, &SettingsDialog::onTreeItemSelected);
    connect(ui->applyButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

SettingsDialog::~SettingsDialog() {
    delete ui;
}

void SettingsDialog::onTreeItemSelected(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    if (!current)
        return;

    bool ok;
    int itemId = current->text(1).toInt(&ok);

    if (ok)
        ui->stackedWidget->setCurrentIndex(itemId);
}
