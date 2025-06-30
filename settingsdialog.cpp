#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QSettings>
#include <QStandardPaths>
#include <QFileDialog>
#include <QStyleFactory>
#include <utility>
#include "lib/bass/include/bass.h"


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

    populateAudioDevices();
    populateLanguages();
    populateThemes();
    populateStyles();

    loadSettings();

    connect(ui->navTree, &QTreeWidget::currentItemChanged, this, &SettingsDialog::onTreeItemSelected);
    connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);
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

    /// Initiative
    int initiativeFields = settings.value(paths.initiative.fields, 7).toInt();
    ui->nameCheckBox->setChecked(initiativeFields & iniFields::name);
    ui->initiativeCheckBox->setChecked(initiativeFields & iniFields::init);
    ui->acCheckBox->setChecked(initiativeFields & iniFields::ac);
    ui->hpCheckBox->setChecked(initiativeFields & iniFields::hp);
    ui->maxhpCheckBox->setChecked(initiativeFields & iniFields::maxHp);
    ui->deleteCheckBox->setChecked(initiativeFields & iniFields::del);
    ui->hpModeComboBox->setCurrentIndex(settings.value(paths.initiative.hpBarMode, 0).toInt());
    ui->showControlCheckBox->setChecked(settings.value(paths.initiative.showHpComboBox, true).toBool());

    QString currentTheme = settings.value(paths.appearance.theme, "Light").toString();
    index = ui->themeComboBox->findData(currentTheme);
    if (index != -1)
        ui->themeComboBox->setCurrentIndex(index);

    QString currentStyle = settings.value(paths.appearance.style, "Fusion").toString();
    index = ui->styleComboBox->findData(currentStyle);
    if (index != -1)
        ui->styleComboBox->setCurrentIndex(index);
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
    settings.setValue(paths.initiative.fields, initiativeFields);
    settings.setValue(paths.initiative.hpBarMode, ui->hpModeComboBox->currentIndex());
    settings.setValue(paths.initiative.showHpComboBox, ui->showControlCheckBox->isChecked());

    settings.setValue(paths.appearance.theme, ui->themeComboBox->currentData().toString());
    settings.setValue(paths.appearance.style, ui->styleComboBox->currentData().toString());
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
    QStringList themeFiles = themesDir.entryList(QStringList() << "*.xml", QDir::Files);

    ui->themeComboBox->addItem("System", "System");
    ui->themeComboBox->addItem("Light", "Light");
    ui->themeComboBox->addItem("Dark", "Dark");

    for (const QString &file: themeFiles) {
        QString themeName = file.mid(0, file.length() - 4);
        ui->themeComboBox->addItem(themeName, file);
    }
}

void SettingsDialog::populateStyles() {
    QStringList styles = QStyleFactory::keys();

    for (const QString& style : styles) {
        ui->styleComboBox->addItem(style, style);
    }
}


