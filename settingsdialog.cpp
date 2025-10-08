#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QSettings>
#include <QToolTip>
#include <QStandardPaths>
#include <QFileDialog>
#include <QMessageBox>
#include <QStyleFactory>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <utility>
#include "lib/bass/include/bass.h"
#include "map-widget/tokenitem.h"
#include "themediconmanager.h"


/**
 * @brief Constructor for the SettingsDialog class.
 *
 * Initializes the Settings dialog with specified application and organisation names
 * and sets up the user interface. It also loads the available audio devices, languages,
 * and themes, and restores saved settings. Connections for UI interactions are established here.
 *
 * @param organisationName The name of the organisation associated with the application.
 * @param applicationName The name of the application.
 * @param parent The parent widget of the settings dialog, if any.
 */
SettingsDialog::SettingsDialog(QString organisationName, QString applicationName, QWidget *parent) :
        QDialog(parent), ui(new Ui::SettingsDialog) {
    ui->setupUi(this);

    m_applicationName = std::move(applicationName);
    m_organisationName = std::move(organisationName);

    ui->navTree->expandAll();
    ui->navTree->setColumnHidden(1, true);
    setupIcons();

    for (auto * edit : ui->hotkeysPage->findChildren<QKeySequenceEdit*>()){
        connect(edit, &QKeySequenceEdit::keySequenceChanged, this, &SettingsDialog::onKeySequenceChanged);
    }

    connect(ui->navTree, &QTreeWidget::currentItemChanged, this, &SettingsDialog::onTreeItemSelected);
    connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->exportButton, &QPushButton::clicked, this, &SettingsDialog::exportSettings);
    connect(ui->importButton, &QPushButton::clicked, this, &SettingsDialog::importSettings);
    connect(ui->masterFogSlider, &QSlider::valueChanged, [=](int value){ui->masterFogValueLabel->setText(QString("%1 \%").arg(value));});
    connect(ui->playerFogSlider, &QSlider::valueChanged, [=](int value){ui->playerFogValueLabel->setText(QString("%1 \%").arg(value));});

    populateAudioDevices();
    populateLanguages();
    populateThemes();
    populateStyles();
    populateTokenModes();

    loadSettings();
}

SettingsDialog::~SettingsDialog() {
    delete ui;
}

/**
 * Handles the event when a tree item in the settings dialog is selected.
 *
 * If the newly selected tree item is valid, this method attempts to parse
 * the item's second column text into an integer. If the parsing succeeds,
 * the stacked widget's current index is updated to match the parsed integer.
 *
 * @param current Pointer to the currently selected tree item. If null, the method returns without making changes.
 * @param previous Pointer to the previously selected tree item. This parameter is unused in the method.
 */
void SettingsDialog::onTreeItemSelected(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    if (!current)
        return;

    bool ok;
    int itemId = current->text(1).toInt(&ok);

    if (ok)
        ui->stackedWidget->setCurrentIndex(itemId);
}

/**
 * @brief Loads the application settings from persistent storage and applies them to the settings dialog.
 *
 * This method reads stored settings using QSettings and updates the respective fields in the settings dialog
 * UI. Settings are grouped based on their purpose, such as general application settings, initiative-related
 * settings, and appearance settings.
 *
 * - General Settings:
 *   - Reads and sets the folder path from persistent storage to UI.
 *   - Selects the default audio device in the UI.
 *   - Sets the application language based on the stored value.
 *
 * - Initiative Settings:
 *   - Retrieves and applies stored initiative field configuration such as name, initiative, AC, HP, max HP,
 *     and delete flags.
 *   - Determines and sets the HP bar display mode.
 *   - Configures whether the HP control combo box should be displayed.
 *
 * - Appearance Settings:
 *   - Reads and applies the currently selected theme.
 */
void SettingsDialog::loadSettings() {
    QSettings settings(m_organisationName, m_applicationName);

    /// General
    ui->folderEdit->setText(settings.value(paths.general.dir, "").toString());
    ui->deviceComboBox->setCurrentIndex(settings.value(paths.general.audioDevice, 0).toInt());
    QString currentLanguage = settings.value(paths.general.lang, "ru_RU").toString();
    int index = ui->languageComboBox->findData(currentLanguage);
    if (index != -1)
        ui->languageComboBox->setCurrentIndex(index);
    ui->startActionComboBox->setCurrentIndex(settings.value(paths.general.startAction, startActions::showEmptyWindow).toInt());
    ui->updateCheckerBox->setChecked(settings.value(paths.general.checkForUpdates, true).toBool());

    /// Initiative
    ui->characterAutoRoll->setChecked(settings.value(paths.initiative.autoInitiative, false).toBool());
    ui->beastAutoRoll->setChecked(settings.value(paths.initiative.beastAutoInitiative, false).toBool());
    int initiativeFields = settings.value(paths.initiative.fields, 7).toInt();
    ui->nameCheckBox->setChecked(initiativeFields & iniFields::name);
    ui->initiativeCheckBox->setChecked(initiativeFields & iniFields::init);
    ui->acCheckBox->setChecked(initiativeFields & iniFields::ac);
    ui->hpCheckBox->setChecked(initiativeFields & iniFields::hp);
    ui->maxhpCheckBox->setChecked(initiativeFields & iniFields::maxHp);
    ui->deleteCheckBox->setChecked(initiativeFields & iniFields::del);
    ui->hpModeComboBox->setCurrentIndex(settings.value(paths.initiative.hpBarMode, 0).toInt());
    ui->showControlCheckBox->setChecked(settings.value(paths.initiative.showHpComboBox, true).toBool());


    /// Appearance
    QString currentTheme = settings.value(paths.appearance.theme, "Light").toString();
    index = ui->themeComboBox->findData(currentTheme);
    if (index != -1)
        ui->themeComboBox->setCurrentIndex(index);

    QString currentStyle = settings.value(paths.appearance.style, "Fusion").toString();
    index = ui->styleComboBox->findData(currentStyle);
    if (index != -1)
        ui->styleComboBox->setCurrentIndex(index);

    ui->scaleSlider->setValue(settings.value(paths.appearance.scale, 100).toInt());

    /// Map
    ui->tokenComboBox->setCurrentIndex(settings.value(paths.map.tokenTitleMode, 0).toInt());
    ui->fontSizeSpinBox->setValue(settings.value(paths.map.tokenFontSize, 12).toInt());
    ui->masterFogSlider->setValue(settings.value(paths.map.masterFogOpacity, 40).toInt());
    ui->playerFogSlider->setValue(settings.value(paths.map.playerFogOpacity, 100).toInt());
    ui->fogColorButton->setText(settings.value(paths.map.fogColor, "#000000").toString());
    ui->lastMapCheckBox->setChecked(settings.value(paths.general.openLastMap, false).toBool());
    ui->gridSizeBox->setValue(settings.value(paths.map.defaultGridSize, 5).toInt());

    /// Hotkeys
    ui->rulerEdit->setKeySequence(QKeySequence(settings.value(paths.hotkeys.ruler).toString()));
    ui->heightEdit->setKeySequence(QKeySequence(settings.value(paths.hotkeys.height).toString()));
    ui->brushEdit->setKeySequence(QKeySequence(settings.value(paths.hotkeys.brush).toString()));
    ui->fogHideEdit->setKeySequence(QKeySequence(settings.value(paths.hotkeys.fogHide).toString()));
    ui->fogRevealEdit->setKeySequence(QKeySequence(settings.value(paths.hotkeys.fogReveal).toString()));
    ui->lightEdit->setKeySequence(QKeySequence(settings.value(paths.hotkeys.light).toString()));
    ui->lightEdit->setKeySequence(QKeySequence(settings.value(paths.hotkeys.light).toString()));
    ui->circleEdit->setKeySequence(QKeySequence(settings.value(paths.hotkeys.circle).toString()));
    ui->squareEdit->setKeySequence(QKeySequence(settings.value(paths.hotkeys.square).toString()));
    ui->triangleEdit->setKeySequence(QKeySequence(settings.value(paths.hotkeys.triangle).toString()));
}

/**
 * @brief Handles the click event for the folder selection button in the settings dialog.
 *
 * This method opens a QFileDialog to allow the user to select a directory. If the user selects
 * a valid directory, the selected folder path is set into the folder edit line in the dialog.
 *
 * The method ensures that:
 * - The initial directory displayed in the QFileDialog matches the text currently in the folder edit field.
 * - If the user cancels the dialog or doesn't select a folder, no changes are made to the dialog's folder edit field.
 */
void SettingsDialog::on_folderButton_clicked() {
    QString folderName = QFileDialog::getExistingDirectory(this,
                                                           tr("Select folder"),
                                                           ui->folderEdit->text().trimmed());
    if (!folderName.isEmpty())
        ui->folderEdit->setText(folderName);
}

/**
 * @brief Slot function triggered when the theme button is clicked.
 *
 * This function allows the user to select a theme file using a file dialog.
 * The selected theme file is then copied to the application's "themes" directory.
 * The theme is added to the theme selection combo box (themeComboBox) with its name
 * and file path as the data.
 *
 * The function performs the following steps:
 * 1. Opens a file dialog for the user to select an XML theme file.
 * 2. Constructs a destination path in the application's "themes" directory.
 * 3. Creates the "themes" directory if it does not exist.
 * 4. Copies the selected theme file to the destination path.
 * 5. Extracts the theme name from the file name (removing the ".xml" extension).
 * 6. Adds the theme name and its file path as an item in the themeComboBox.
 */
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

/**
 * @brief Saves the current settings to a persistent QSettings storage.
 *
 * This function saves various user-configurable settings, such as general application
 * preferences, initiative configuration, and appearance settings, using the QSettings
 * class. The settings are written under the organization and application name associated
 * with this instance of SettingsDialog.
 *
 * The saved settings include:
 * - General settings:
 *    - Selected audio device index
 *    - Default directory path
 *    - Selected application language
 * - Initiative settings:
 *    - Enabled fields for initiative view (e.g., name, initiative, AC, HP, max HP, delete)
 *    - HP bar mode setting
 *    - Show HP control visibility
 * - Appearance settings:
 *    - Selected application theme
 *
 * The function retrieves values from the user interface elements and stores them in the
 * corresponding paths provided by the `paths` member variable. After saving all settings,
 * the function ensures they are written to the persistent storage by calling `QSettings::sync()`.
 */
void SettingsDialog::saveSettings() {
    QSettings settings(m_organisationName, m_applicationName);
    /// General
    settings.setValue(paths.general.audioDevice, deviceIndices[ui->deviceComboBox->currentIndex()]);
    settings.setValue(paths.general.dir, ui->folderEdit->text());
    settings.setValue(paths.general.lang, ui->languageComboBox->currentData().toString());
    settings.setValue(paths.general.startAction, ui->startActionComboBox->currentIndex());
    settings.setValue(paths.general.checkForUpdates, ui->updateCheckerBox->isChecked());

    /// Initiative
    int initiativeFields = 0;
    if (ui->nameCheckBox->isChecked())
        initiativeFields = initiativeFields + iniFields::name;
    if (ui->initiativeCheckBox->isChecked())
        initiativeFields = initiativeFields + iniFields::init;
    if (ui->acCheckBox->isChecked())
        initiativeFields = initiativeFields + iniFields::ac;
    if (ui->hpCheckBox->isChecked())
        initiativeFields = initiativeFields + iniFields::hp;
    if (ui->maxhpCheckBox->isChecked())
        initiativeFields = initiativeFields + iniFields::maxHp;
    if (ui->deleteCheckBox->isChecked())
        initiativeFields = initiativeFields + iniFields::del;
    settings.setValue(paths.initiative.fields, initiativeFields);
    settings.setValue(paths.initiative.hpBarMode, ui->hpModeComboBox->currentIndex());
    settings.setValue(paths.initiative.showHpComboBox, ui->showControlCheckBox->isChecked());
    settings.setValue(paths.initiative.autoInitiative, ui->characterAutoRoll->isChecked());
    settings.setValue(paths.initiative.beastAutoInitiative, ui->beastAutoRoll->isChecked());

    /// Appearance
    settings.setValue(paths.appearance.theme, ui->themeComboBox->currentData().toString());
    settings.setValue(paths.appearance.style, ui->styleComboBox->currentData().toString());
    settings.setValue(paths.appearance.scale, ui->scaleSlider->value());

    /// Map
    settings.setValue(paths.map.tokenTitleMode, ui->tokenComboBox->currentIndex());
    settings.setValue(paths.map.tokenFontSize, ui->fontSizeSpinBox->value());
    settings.setValue(paths.map.masterFogOpacity, ui->masterFogSlider->value());
    settings.setValue(paths.map.playerFogOpacity, ui->playerFogSlider->value());
    settings.setValue(paths.map.fogColor, ui->fogColorButton->text());
    settings.setValue(paths.map.defaultGridSize, ui->gridSizeBox->value());
    settings.setValue(paths.general.openLastMap, ui->lastMapCheckBox->isChecked());

    /// Hotkeys
    settings.setValue(paths.hotkeys.ruler, ui->rulerEdit->keySequence().toString());
    settings.setValue(paths.hotkeys.height, ui->heightEdit->keySequence().toString());
    settings.setValue(paths.hotkeys.brush, ui->brushEdit->keySequence().toString());
    settings.setValue(paths.hotkeys.fogHide, ui->fogHideEdit->keySequence().toString());
    settings.setValue(paths.hotkeys.fogReveal, ui->fogRevealEdit->keySequence().toString());
    settings.setValue(paths.hotkeys.light, ui->lightEdit->keySequence().toString());
    settings.setValue(paths.hotkeys.line, ui->lineEdit->keySequence().toString());
    settings.setValue(paths.hotkeys.circle, ui->circleEdit->keySequence().toString());
    settings.setValue(paths.hotkeys.square, ui->squareEdit->keySequence().toString());
    settings.setValue(paths.hotkeys.triangle, ui->triangleEdit->keySequence().toString());

    settings.sync();
}

/**
 * Populates a list of audio devices and updates the associated combo box in the UI.
 *
 * This function retrieves information about available audio playback devices using the BASS library.
 * It clears any existing data in the `deviceNames` and `deviceIndices` containers, as well as the
 * UI combo box used for selecting devices. Enabled devices are identified, their names are added
 * to `deviceNames`, their indices are added to `deviceIndices`, and they are displayed in the
 * combo box. If any devices are found, the combo box selection defaults to the first device.
 *
 * Device information is fetched using `BASS_GetDeviceInfo`, which populates the `BASS_DEVICEINFO`
 * structure for each device.
 *
 * Preconditions:
 * - The `ui` pointer must be initialized and point to a valid `Ui::SettingsDialog` object.
 *
 * Postconditions:
 * - The `deviceNames` list will contain the names of all enabled audio devices.
 * - The `deviceIndices` vector will contain the indices corresponding to the audio devices.
 * - The `deviceComboBox` in the UI will be updated to display the names of the audio devices.
 * - If available devices are found, the first device will be selected in the combo box.
 */
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

/**
 * @brief Handles the `apply` button click event by saving settings and closing the dialog.
 *
 * This function is called when the `apply` button in the settings dialog is clicked.
 * It performs the following actions:
 * - Calls `saveSettings()` to store the current user-configurable settings into persistent
 *   storage, including general, initiative, and appearance settings.
 * - Accepts the dialog by internally calling the `accept()` function, which closes
 *   the dialog and emits the `accepted()` signal to notify that the dialog was confirmed.
 */
void SettingsDialog::on_applyButton_clicked() {
    if (!validateKeySequences()){
        ui->applyButton->setEnabled(false);
        ui->stackedWidget->setCurrentIndex(settingsPages::hotkeys);
        return;
    }
    saveSettings();
    accept();
}

/**
 * @brief Populates the language selection combo box with available translation languages.
 *
 * This method scans the "translations" subdirectory within the application's directory
 * for translation files following the naming pattern "dm-assist_*.qm". It extracts the
 * locale information from the filenames, translates them into the native language names,
 * and adds them as items to the combo box `ui->languageComboBox`. Each combo box item
 * is associated with the corresponding locale string as user data. The items in the
 * combo box are then sorted alphabetically for convenience.
 *
 * Language files must be named following the pattern: "dm-assist_cc.qm",
 * where "cc" represents the language code (ISO 639-1).
 */
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

/**
 * Populates the theme selection combo box with available themes.
 *
 * This method first adds default themes ("Light" and "Dark") to the combo box.
 * Then, it scans the "themes" directory located in the application's directory
 * for XML files representing additional themes. For each valid theme file found,
 * the method extracts its name (by removing the ".xml" extension) and adds it
 * to the combo box as an item.
 *
 * The theme names become visible in the combo box, while the corresponding file names
 * are stored as the associated data for each item.
 */
void SettingsDialog::populateThemes() {
    QDir themesDir(QCoreApplication::applicationDirPath() + "/themes");
    QFileInfoList themeFilesInfo = themesDir.entryInfoList(QStringList() << "*.xml", QDir::Files);

    ui->themeComboBox->addItem("System", "System");
    ui->themeComboBox->addItem("Light", "Light");
    ui->themeComboBox->addItem("Dark", "Dark");

    for (const QFileInfo &info: themeFilesInfo) {
        ui->themeComboBox->addItem(info.completeBaseName(), info.absoluteFilePath());
    }
}

void SettingsDialog::populateStyles() {
    QStringList styles = QStyleFactory::keys();

    for (const QString& style : styles) {
        ui->styleComboBox->addItem(style, style);
    }
}

void SettingsDialog::populateTokenModes() {
    for (int i = 0; i <= TokenTitleDisplayMode::noTitle; i++){
        ui->tokenComboBox->addItem(TokenItem::stringMode(i));
    }
}

void SettingsDialog::onKeySequenceChanged(QKeySequence newSeq) {
    auto *editSender = qobject_cast<QKeySequenceEdit*>(sender());
    if (!editSender) return;

    QString key = newSeq.toString(QKeySequence::NativeText);
    m_hotkeyHash[editSender] = key;

    ui->applyButton->setEnabled(validateKeySequences());
}

bool SettingsDialog::validateKeySequences() {
    bool ok = true;
    QHash<QString, int> countHash;
    for (auto it = m_hotkeyHash.begin(); it != m_hotkeyHash.end(); ++it) {
        if (!it.value().isEmpty())
            countHash[it.value()]++;
    }

    for (auto it = m_hotkeyHash.begin(); it != m_hotkeyHash.end(); ++it) {
        if (countHash[it.value()] > 1) {
            it.key()->setStyleSheet("color: #FF0000"); ///< Error
            ok = false;
        } else {
            it.key()->setStyleSheet("color: palette(text)"); ///< OK
        }
    }
    return ok;
}

void SettingsDialog::setupIcons() {
    ThemedIconManager::instance().addPixmapTarget(":/map/ruler.svg", ui->rulerIcon, [label = ui->rulerIcon](const QPixmap& px){label->setPixmap(px);});
    ThemedIconManager::instance().addPixmapTarget(":/map/mountain.svg", ui->heightIcon, [label = ui->heightIcon](const QPixmap& px){label->setPixmap(px);});
    ThemedIconManager::instance().addPixmapTarget(":/map/brush.svg", ui->brushIcon, [label = ui->brushIcon](const QPixmap& px){label->setPixmap(px);});
    ThemedIconManager::instance().addPixmapTarget(":/map/fog_hide.svg", ui->fogHideIcon, [label = ui->fogHideIcon](const QPixmap& px){label->setPixmap(px);});
    ThemedIconManager::instance().addPixmapTarget(":/map/fog_reveal.svg", ui->fogRevealIcon, [label = ui->fogRevealIcon](const QPixmap& px){label->setPixmap(px);});
    ThemedIconManager::instance().addPixmapTarget(":/map/torch.svg", ui->lightIcon, [label = ui->lightIcon](const QPixmap& px){label->setPixmap(px);});
    ThemedIconManager::instance().addPixmapTarget(":/map/line.svg", ui->lineIcon, [label = ui->lineIcon](const QPixmap& px){label->setPixmap(px);});
    ThemedIconManager::instance().addPixmapTarget(":/map/sphere.svg", ui->circleIcon, [label = ui->circleIcon](const QPixmap& px){label->setPixmap(px);});
    ThemedIconManager::instance().addPixmapTarget(":/map/cube.svg", ui->squareIcon, [label = ui->squareIcon](const QPixmap& px){label->setPixmap(px);});
    ThemedIconManager::instance().addPixmapTarget(":/map/cone.svg", ui->triangleIcon, [label = ui->triangleIcon](const QPixmap& px){label->setPixmap(px);});
}


void SettingsDialog::exportSettings() {
    QString file = QFileDialog::getSaveFileName(this,
                                                tr("Export settings to file"),
                                                QString(),
                                                tr("XML files (*.xml)"));

    if (file.isEmpty()) return;

    QSettings settings(m_organisationName, m_applicationName);
    XmlSettingsWriter writer(this);
    writer.excludeKeys(m_excludedKeys);
    QString error;
    if (!writer.exportToFile(file, settings, &error))
        QMessageBox::critical(this,
                              tr("Export failed"),
                              tr("failed to export settings: \n%1").arg(error));
    else
        QMessageBox::information(this,
                                 tr("Export finished"),
                                 tr("Settings exported to \n%1").arg(file));
}

void SettingsDialog::importSettings() {
    QString file = QFileDialog::getOpenFileName(this,
                                                tr("Import settings from file"),
                                                QString(),
                                                tr("XML files (*.xml)"));
    if (file.isEmpty()) return;
    QString backup = QDir::homePath() + "/dm-assist-settings-backup-" + QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss") + ".xml";

    auto res = QMessageBox::question(this, tr("Import settings"),
                                     tr("Import will overwrite existing settings \n"
                                        "Current settings backup will be saved to: \n %1 \n\n"
                                        "Do you want to import settings?").arg(backup));
    if (res == QMessageBox::No) return;


    exportSettingsToFile(backup);
    QSettings settings(m_organisationName, m_applicationName);
    XmlSettingsWriter importer(this);
    QString err;
    if (!importer.importFromFile(file, settings, &err))
        QMessageBox::critical(this,
                              tr("Import failed"),
                              tr("Failed to import settings \n %1").arg(err));
    else
        QMessageBox::information(this,
                                 tr("Import finished"),
                                 tr("Settings imported successfully from \n%1").arg(file));
    settings.sync();
    on_applyButton_clicked();
}

bool SettingsDialog::exportSettingsToFile(QString &filePath) {
    if (filePath.isEmpty()) return false;

    QSettings settings(m_organisationName, m_applicationName);
    XmlSettingsWriter writer(this);
    writer.excludeKeys(m_excludedKeys);
    return writer.exportToFile(filePath, settings);
}

bool SettingsDialog::importSettingsFromFile(QString &filePath) {
    if (filePath.isEmpty()) return false;

    QSettings settings(m_organisationName, m_applicationName);
    XmlSettingsWriter importer(this);
    return importer.importFromFile(filePath, settings);
}

QString XmlSettingsWriter::serializeVariant(const QVariant &v) const {
    if (v.typeId() == QMetaType::QStringList)
        return v.toStringList().join(";");
    return v.toString();
}

QVariant XmlSettingsWriter::deserializeVariant(const QString &text, const QString &type) const {
    if (type == "bool" || type == "QBool")
        return QVariant(text.toLower() == "true" || text == "1");
    if (type == "int" || type == "QInt"){
        bool ok = false;
        int x = text.toInt(&ok);
        return ok ? QVariant(x) : QVariant(text);
    }
    if (type == "double" || type == "QDouble") {
        bool ok = false;
        double d = text.toDouble(&ok);
        return  ok ? QVariant(d) : QVariant(text);
    }
    if (type == "QStringList")
        return QVariant(text.split(";", Qt::SkipEmptyParts));

    return QVariant(text);
}

bool XmlSettingsWriter::exportToFile(const QString &filePath, QSettings &settings, QString *errorString) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)){
        if (errorString) *errorString = file.errorString();
        return false;
    }

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("settings");
    xml.writeAttribute("version", SETTINGS_VERSION);

    const QStringList keys = settings.allKeys();

    for (const QString& key : keys) {
        if (m_excludedKeys.contains(key))
            continue;
        QVariant v = settings.value(key);
        QString typeName = v.typeName();
        QString serialized = serializeVariant(v);

        xml.writeStartElement("entry");
        xml.writeAttribute("key", key);
        xml.writeAttribute("type", typeName);
        xml.writeCharacters(serialized);
        xml.writeEndElement(); // entry
    }
    xml.writeEndElement(); // settings
    xml.writeEndDocument();
    file.close();
    return true;
}

bool XmlSettingsWriter::importFromFile(const QString &filePath, QSettings &settings, QString *errorString) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)){
        if (errorString) *errorString = file.errorString();
        return false;
    }
    QXmlStreamReader xml(&file);

    while (!xml.atEnd() && !xml.hasError()){
        QXmlStreamReader::TokenType token = xml.readNext();

        if (token == QXmlStreamReader::StartElement){
            if (xml.name().toString() == "entry"){
                QXmlStreamAttributes attributes = xml.attributes();
                QString key = attributes.value("key").toString();
                QString type = attributes.value("type").toString();
                QString textValue = xml.readElementText(QXmlStreamReader::SkipChildElements);

                QVariant v = deserializeVariant(textValue, type);
                settings.setValue(key, v);
            }
        }
    }
    if (xml.hasError()){
        if (errorString) *errorString = xml.errorString();
        file.close();
        return false;
    }
    file.close();
    settings.sync();
    return true;
}

void XmlSettingsWriter::excludeKeys(const QStringList &keys) {
    m_excludedKeys = keys;
}

void XmlSettingsWriter::addExcludedKeys(const QStringList &addKeys) {
    m_excludedKeys.append(addKeys);
}
