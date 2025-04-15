//
// Created by arseniy on 02.04.2025.
//

#include "settingsdialog.h"
#include "ui_SettingsDialog.h"

#include <QSettings>
#include <QStandardPaths>
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
    populateThemes();

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

    QString currentLanguage = settings.value(paths.general.lang, "ru_RU").toString();
    int index = ui->languageComboBox->findData(currentLanguage);
    if (index != -1)
        ui->languageComboBox->setCurrentIndex(index);

    QString currentTheme = settings.value(paths.appearance.theme, "Light").toString();
    index = ui->themeComboBox->findData(currentTheme);
    if (index != -1)
        ui->themeComboBox->setCurrentIndex(index);
}

void SettingsDialog::on_folderButton_clicked() {
    QString folderName = QFileDialog::getExistingDirectory(this,
                                                           tr("Select folder"),
                                                           ui->folderEdit->text().trimmed());
    if (!folderName.isEmpty())
        ui->folderEdit->setText(folderName);
}

void SettingsDialog::on_themeButton_clicked() {
    QString themeFileName = QFileDialog::getOpenFileName(this,
                                                         tr("Select theme file"),
                                                         QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
                                                         "Xml file (*.xml)");
    QFileInfo info(themeFileName);
    QString dest = QCoreApplication::applicationDirPath() + "/themes/" + info.fileName();

    QDir destDir(QCoreApplication::applicationDirPath() + "/themes");
    if (!destDir.exists())
        destDir.mkpath(".");
    QFile::copy(themeFileName, dest);

    QString themeName = dest.mid(0, dest.length() - 4);
    ui->themeComboBox->addItem(themeName, dest);

}

void SettingsDialog::saveSettings() {
    QSettings settings(m_organisationName, m_applicationName);
    settings.setValue(paths.general.audioDevice, deviceIndices[ui->deviceComboBox->currentIndex()]);
    settings.setValue(paths.general.dir, ui->folderEdit->text());
    settings.setValue(paths.general.lang, ui->languageComboBox->currentData().toString());
    settings.setValue(paths.appearance.theme, ui->themeComboBox->currentData().toString());
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
        QString locale = file.mid(10, file.length() - 13); // "dm-assist_" (10 chars) + ".qm" (3 chars)
        QLocale ql(locale.mid(0, 2));
        QString langName = ql.nativeLanguageName();
        ui->languageComboBox->addItem(langName, locale);
    }
    ui->languageComboBox->model()->sort(0);
}

void SettingsDialog::populateThemes() {
    QDir themesDir(QCoreApplication::applicationDirPath() + "/themes");
    QStringList themeFiles = themesDir.entryList(QStringList() << "*.xml", QDir::Files);

    ui->themeComboBox->addItem("Light", "Light");
    ui->themeComboBox->addItem("Dark", "Dark");

    for (const QString &file: themeFiles) {
        QString themeName = file.mid(0, file.length() - 4);
        ui->themeComboBox->addItem(themeName, file);
    }
}


