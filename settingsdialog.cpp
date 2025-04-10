//
// Created by arseniy on 02.04.2025.
//

#include "settingsdialog.h"
#include "ui_SettingsDialog.h"

#include <QSettings>
#include <QFileDialog>
#include <utility>
#include <QDebug>
#include "lib/bass/bass.h"


SettingsDialog::SettingsDialog(QString organisationName, QString applicationName, QWidget *parent) :
        QDialog(parent), ui(new Ui::SettingsDialog) {
    ui->setupUi(this);

    m_applicationName = std::move(applicationName);
    m_organisationName = std::move(organisationName);

    ui->navTree->expandAll();
    ui->navTree->setColumnHidden(1, true);

    populateAudioDevices();
    loadSettings();

    connect(ui->navTree, &QTreeWidget::currentItemChanged, this, &SettingsDialog::onTreeItemSelected);
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

void SettingsDialog::loadSettings() {

    QSettings settings(m_organisationName, m_applicationName);

    ui->folderEdit->setText(settings.value(paths.general.dir, "").toString());
    ui->deviceComboBox->setCurrentIndex(settings.value(paths.general.audioDevice, 0).toInt());
}

void SettingsDialog::on_folderButton_clicked() {
    QString folderName = QFileDialog::getExistingDirectory(this,
                                                           "Выберите папку",
                                                           ui->folderEdit->text().trimmed());
    if (!folderName.isEmpty())
        ui->folderEdit->setText(folderName);
}

void SettingsDialog::saveSettings() {
    QSettings settings(m_organisationName, m_applicationName);
    settings.setValue(paths.general.audioDevice, deviceIndices[ui->deviceComboBox->currentIndex()]);
    settings.setValue(paths.general.dir, ui->folderEdit->text());
    settings.sync();
    accept();
}

void SettingsDialog::populateAudioDevices() {
    deviceNames.clear();
    deviceIndices.clear();
    ui->deviceComboBox->clear();

    BASS_DEVICEINFO info;

    for (int i = 0; BASS_GetDeviceInfo(i, &info); i++) {
        if (info.flags & BASS_DEVICE_ENABLED)
        {
            QString deviceName = QString::fromLocal8Bit(info.name);
            deviceNames << deviceName;
            deviceIndices << i;
            ui->deviceComboBox->addItem(deviceName);
        }
    }
    if (!deviceIndices.isEmpty())
    {
        ui->deviceComboBox->setCurrentIndex(0);
    }
}

void SettingsDialog::on_applyButton_clicked() {
    saveSettings();
}


