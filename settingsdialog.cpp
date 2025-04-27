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
    populateLanguages();

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

    /// General
    ui->folderEdit->setText(settings.value(paths.general.dir, "").toString());
    ui->deviceComboBox->setCurrentIndex(settings.value(paths.general.audioDevice, 0).toInt());
    QString currentLanguage = settings.value(paths.general.lang, "ru_RU").toString();
    int index = ui->languageComboBox->findData(currentLanguage);
    if (index != -1)
        ui->languageComboBox->setCurrentIndex(index);

    /// Initiative
    int initiativeFields = settings.value(paths.inititiative.fields, 7).toInt();
    ui->nameCheckBox->setChecked(initiativeFields & iniFields::name);
    ui->initiativeCheckBox->setChecked(initiativeFields & iniFields::initiative);
    ui->acCheckBox->setChecked(initiativeFields & iniFields::ac);
    ui->hpCheckBox->setChecked(initiativeFields & iniFields::hp);
    ui->maxhpCheckBox->setChecked(initiativeFields & iniFields::maxHp);
    ui->deleteCheckBox->setChecked(initiativeFields & iniFields::del);
    ui->hpModeComboBox->setCurrentIndex(settings.value(paths.inititiative.hpBarMode, 0).toInt());
    ui->showControlCheckBox->setChecked(settings.value(paths.inititiative.showHpComboBox, true).toBool());
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
    /// General
    settings.setValue(paths.general.audioDevice, deviceIndices[ui->deviceComboBox->currentIndex()]);
    settings.setValue(paths.general.dir, ui->folderEdit->text());
    settings.setValue(paths.general.lang, ui->languageComboBox->currentData().toString());

    /// Initiative
    int initiativeFields = 0;
    if (ui->nameCheckBox->isChecked())
        initiativeFields = initiativeFields + 1;
    if (ui->initiativeCheckBox->isChecked())
        initiativeFields = initiativeFields + 2;
    if (ui->acCheckBox->isChecked())
        initiativeFields = initiativeFields + 4;
    if (ui->hpCheckBox->isChecked())
        initiativeFields = initiativeFields + 8;
    if (ui->maxhpCheckBox->isChecked())
        initiativeFields = initiativeFields + 16;
    if (ui->deleteCheckBox->isChecked())
        initiativeFields = initiativeFields + 32;
    settings.setValue(paths.inititiative.fields, initiativeFields);
    settings.setValue(paths.inititiative.hpBarMode, ui->hpModeComboBox->currentIndex());
    settings.setValue(paths.inititiative.showHpComboBox, ui->showControlCheckBox->isChecked());

    settings.sync();
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
    accept();
}

void SettingsDialog::populateLanguages() {
    QDir langDir(QCoreApplication::applicationDirPath() + "/translations");
    QStringList qmFiles = langDir.entryList(QStringList() << "dm-assist_*.qm", QDir::Files);

    for (const QString &file : qmFiles)
    {
        QString locale = file.mid(10, file.length() - 13); // "dm-assist" (9 chars) + ".qm" (3 chars)
        QLocale ql(locale.mid(0, 2));
        QString langName = ql.nativeLanguageName();
        ui->languageComboBox->addItem(langName, locale);
    }
    ui->languageComboBox->model()->sort(0);
}


