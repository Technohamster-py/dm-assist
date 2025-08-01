#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "map-widget/mapview.h"
#include "charsheet-widget/dndcharsheetwidget.h"

#include <QDesktopServices>
#include "QDomDocument"
#include <QFile>
#include <QFileDialog>
#include <QColorDialog>
#include <QCheckBox>
#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <QMimeData>
#include "saveconfigdialog.h"
#include <QStyleFactory>
#include <QTextStream>
#include <QTextBrowser>
#include <QSpinBox>
#include <QJsonDocument>

#include <QDebug>
#include "theme-manager/thememanager.h"
#include "theme-manager/themediconmanager.h"

#include <QDebug>

static void copyAllFiles(const QString& sourcePath, const QString& destPath);
static void moveAllFiles(const QString& sourcePath, const QString& destPath);
static bool removeDirectoryRecursively(const QString &directoryPath, bool deleteSelf=true);


/**
 * @brief Constructor for the MainWindow class.
 *
 * Initializes a new instance of the MainWindow class, setting up the
 * UI and establishing the parent-child relationship with the provided parent widget.
 *
 * @param parent Pointer to the parent QWidget. Defaults to nullptr if not provided,
 * which means the MainWindow has no parent and will be a top-level window.
 */
MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    updateChecker = new UpdateChecker(VERSION, RELEASES_URL, this);
    ui->updateBanner->hide();

    rollWidget = new RollWidget(ui->leftAsideWidget);
    connect(this, &MainWindow::translatorChanged, rollWidget, &RollWidget::updateTranslator);
    ui->rollLayout->addWidget(rollWidget);

    campaignTreeWidget = new CampaignTreeWidget(ui->leftAsideWidget);
    campaignTreeWidget->setVisible(false);
    setupCampaign(QString());
    setupToolbar();
    setupPlayers();
    setupTracker();
    setupMaps();

    adjustSize();

    setAcceptDrops(true);

    setupShortcuts();

    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newCampaign);
    connect(ui->actionClose, &QAction::triggered, this, &MainWindow::closeCampaign);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::loadCampaign);
    connect(ui->actionHelp, &QAction::triggered, [](){QDesktopServices::openUrl(QUrl(HELP_URL));});
    connect(ui->actionDonate, &QAction::triggered, [](){QDesktopServices::openUrl(QUrl(DONATE_URL));});
    connect(ui->actionReport_bug, &QAction::triggered, [](){QDesktopServices::openUrl(QUrl(ISSUES_URL));});
    connect(ui->actionSources, &QAction::triggered, [=]() { showSourcesMessageBox(sourcesMap);});
    connect(ui->volumeSlider, &QSlider::valueChanged, this, &MainWindow::setVolumeDivider);
    connect(ui->actionReload, &QAction::triggered, [=](){setupCampaign(campaignTreeWidget->root());});
    connect(ui->actionAddCharacter, &QAction::triggered, this, &MainWindow::addCharacter);
    connect(ui->actionCheck, &QAction::triggered, [=](){updateChecker->checkFotUpdates();});
    connect(updateChecker, &UpdateChecker::updateCheckFinished, this, &MainWindow::handleUpdates);

    connect(campaignTreeWidget, &CampaignTreeWidget::characterAddRequested, this, [=](const QString& path) {
        DndCharsheetWidget character(path);
        character.addToInitiative(initiativeTrackerWidget, autoRoll);
    });
    connect(campaignTreeWidget, &CampaignTreeWidget::characterOpenRequested, this, [=](const QString& path){
        auto* charsheetWidget = new DndCharsheetWidget(path);
        connect(charsheetWidget, &DndCharsheetWidget::rollRequested, rollWidget, &RollWidget::executeRoll);
        connect(this, &MainWindow::translatorChanged, charsheetWidget, &DndCharsheetWidget::updateTranslator);
        charsheetWidget->show();
    });
    connect(campaignTreeWidget, &CampaignTreeWidget::mapOpenRequested, this, &MainWindow::openMapFromFile);
    ui->campaignLayout->addWidget(campaignTreeWidget);

    ThemedIconManager::instance().addIconTarget<QAbstractButton>(":/player/Volume-1.svg", ui->muteButton, &QAbstractButton::setIcon);
    ThemedIconManager::instance().addPixmapTarget(":/player/Volume-2.svg", ui->volMaxLabel, [label = ui->volMaxLabel](const QPixmap& px){label->setPixmap(px);});

    showMaximized();

    loadSettings();
    saveSettings();

    updateChecker->checkFotUpdates();
}

/**
 * Закрытие главного окна
 * @details При закрытии окна производится сохранение конфигурационного файла
 */
MainWindow::~MainWindow() {
    closeCampaign();
    saveSettings();
    foreach(MusicPlayerWidget* player, players){
        removeDirectoryRecursively(player->getLocalDirPath());
        delete player;
    }
    delete ui;
}

/**
 * @brief Changes the application's language to the specified language code.
 *
 * This function updates the application's current language by removing the existing
 * translator, loading the translation file for the specified language code, and
 * installing the new translator. If the translation file is successfully loaded,
 * the UI will be retranslated to reflect the language change.
 *
 * @param languageCode The language code (e.g., "en", "fr") for the desired language.
 *
 * The language file is expected to be located in the "translations" directory
 * relative to the application's directory and should follow the naming convention
 * "dm-assist_<languageCode>.qm".
 */
void MainWindow::changeLanguage(const QString &languageCode) {
    qApp->removeTranslator(&translator);
    if (translator.load(QCoreApplication::applicationDirPath() + "/translations/dm-assist_" + languageCode + ".qm"))
    {
        qApp->installTranslator(&translator);
        currentLanguage = languageCode;
        ui->retranslateUi(this);
        emit translatorChanged();
    }
}

/**
 * @brief Toggles the visibility of a fog layer on the currently active map scene.
 *
 * This function retrieves the currently active map view from the `mapTabWidget`
 * and accesses its associated `MapScene`. Depending on the value of the `hide`
 * parameter, it either hides or reveals all fog elements on the map scene using
 * the `fogTool`.
 *
 * @param hide If true, hides the fog layer completely; if false, reveals it.
 */
void MainWindow::coverMapWithFog(bool hide) {
    auto* currentView = qobject_cast<MapView*>(mapTabWidget->currentWidget());
    MapScene* scene = currentView->getScene();

    if (hide)
        fogTool->hideAll(scene);
    else
        fogTool->revealAll(scene);
}

/**
 * @brief Creates a new map tab in the application.
 *
 * This function opens a file dialog to allow the user to select a map file. The file can either be
 * a ".dam" file (map scene file) or an image file (e.g., ".png", ".jpg", ".bmp"). If the user selects
 * a valid file, it creates a new MapView instance, attempts to load the file into the view, and adds
 * it as a new tab in the mapTabWidget if successful.
 *
 * If the file is a ".dam" file, it loads the scene data using MapView::loadSceneFromFile().
 * If the file is an image, it loads the image into the map using MapView::loadMapImage().
 *
 * If the file loading fails, an error dialog is displayed, and the MapView instance is deleted.
 * Otherwise, the tab is added, visibility of the mapTabWidget is updated, the new tab is selected,
 * and tool change handling is connected for the new MapView instance.
 *
 * The toolChanged signal from the MapScene associated with the MapView is connected to a lambda
 * function that ensures appropriate QAction items in the toolGroup are checked or unchecked based
 * on the active tool.
 */
void MainWindow::createNewMapTab() {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Map Image"),
                                                    QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                                                    tr("Map Files (*.dam);;Images (*.png *.jpg *.bmp)"));

    if (fileName.isEmpty()) return;

    openMapFromFile(fileName);
}

/**
 * @brief Deletes a specific map tab from the application's tab widget.
 *
 * This function removes a tab from the `mapTabWidget` based on the given index,
 * deletes the associated `QWidget` to free up resources, and then updates the visibility
 * of certain UI elements based on the remaining tabs.
 *
 * @param index The index of the tab to be deleted. It should be a valid index within the range of existing tabs.
 *
 * @note Calling this function on an invalid index (e.g., out of range) can result in undefined behavior.
 */
void MainWindow::deleteMapTab(int index) {
    QWidget *widget = mapTabWidget->widget(index);

    if (!currentCampaignDir.isEmpty()){
        exportMap(currentCampaignDir + "/Maps/" + mapTabWidget->tabText(index) + ".dam", index);
    }

    mapTabWidget->removeTab(index);
    delete widget;
    updateVisibility();
}

/**
 * @brief Exports the map from the specified tab index to a file.
 *
 * This function allows the user to export the current map displayed in the specified
 * tab of the mapTabWidget to a file. A file save dialog is presented to the user to
 * select the location and name of the output file. The map data is saved in a proprietary
 * file format (*.dam).
 *
 * @param index The index of the tab in mapTabWidget containing the map to be exported.
 *
 * @note If the specified tab does not contain a valid MapView object, the operation is skipped.
 *       The exported file format is DM assist map file (*.dam).
 */
void MainWindow::slotExportMap(int index) {
    auto* currentView = qobject_cast<MapView*>(mapTabWidget->widget(index));
    if (currentView){
        QString filename = QFileDialog::getSaveFileName(this,
                                                        tr("Save map to file"),
                                                        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                        "DM assist map file (*.dam)");
        currentView->getScene()->saveToFile(filename);
    }
}

/**
 * @brief Exports the map from a specified tab to a file.
 *
 * This function is responsible for exporting the map data from a given tab
 * of the map tab widget to a file. The file format is expected to be ".dam".
 * If the provided file path does not have the ".dam" extension, the function
 * exits without performing any operation.
 *
 * The target tab is identified by its index in the map tab widget. If the
 * widget at the specified index is not a valid `MapView`, the function will
 * not perform any actions.
 *
 * If the widget is a valid `MapView`, the function retrieves the associated
 * `MapScene` object and invokes its `saveToFile` method to save the map
 * data to the specified file path.
 *
 * @param path The file path where the map data should be exported. It must end with ".dam".
 * @param index The index of the tab in the map tab widget containing the map to export.
 */
void MainWindow::exportMap(const QString& path, int index) {
    if (!path.endsWith(".dam")) return;
    
    auto* currentView = qobject_cast<MapView*>(mapTabWidget->widget(index));
    if (currentView){
        currentView->getScene()->saveToFile(path);
    }
}

/**
 * @brief Loads a campaign by allowing the user to select a campaign directory and initializing it.
 *
 * This function opens a file dialog to let the user select a campaign directory.
 * Once a valid directory is selected, it calls the `setupCampaign` function to configure and
 * initialize the campaign environment. The selected directory is set as the currently active campaign directory.
 *
 * Behavior:
 * - Displays a file dialog for the user to choose a directory.
 * - Utilizes `QStandardPaths::DocumentsLocation` as the default starting location for the dialog.
 * - If a directory is selected, it passes the directory path to `setupCampaign` for initializing
 *   the campaign environment.
 */
void MainWindow::loadCampaign() {
    QString campaignDir = QFileDialog::getExistingDirectory(this,
                                                            tr("Open campaign directory"),
                                                            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    setupCampaign(campaignDir);
}

/**
 * @brief Loads and parses a configuration XML file for players' playlists.
 *
 * This function displays a file open dialog to the user, allowing them to select a configuration XML file.
 * If a valid file is selected, the file is opened and read. The XML structure is parsed to extract playlist data
 * for the players. Each player node in the XML specifies the ID of the player and the associated folder containing
 * playlist files. The function handles the following:
 *
 * - Extracting the absolute path to the playlist folder from the file.
 * - Validating that the specified folder exists.
 * - Setting the name of the playlist for each player based on the folder name.
 * - Adding media files from the folder to the respective player's playlist.
 *
 * Error dialogs are shown if the file fails to open or if the folder specified does not exist.
 * The function ensures players are updated with appropriate data or left empty if no valid data is found.
 */
void MainWindow::loadMusicConfigFile(QString fileName) {
    if (fileName.isEmpty())
        fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open player config file"),
                                                    QStandardPaths::writableLocation(QStandardPaths::MusicLocation) + "/dm_assist_files/saves",
                                                    tr("Xml file (*.xml)"));
    if(fileName.isEmpty())
        return;
    else
    {
        QFile configFile(fileName);
        if (!configFile.open(QIODevice::ReadOnly))
        {
            QMessageBox::critical(this, tr("Open File error"), configFile.errorString());
            return;
        }
        QDomDocument configDocument;
        configDocument.setContent(&configFile);

        QDomElement mainNode = configDocument.documentElement();
        QDomNodeList playersNodeList = mainNode.childNodes();

        for (int i=0; i < playersNodeList.count(); i++){
            QDomElement playerNode = playersNodeList.at(i).toElement();
            QString absolutePath = playerNode.firstChild().toText().data();

            int playerId = playerNode.attribute("id").toInt();

            players[playerId]->setVolume(playerNode.attribute("volume", "100").toInt());

            QFileInfo dirInfo(absolutePath);
            if (dirInfo.isDir()){
                players[playerId]->setPlaylistName(dirInfo.fileName());
            }

            QDir playerDir(absolutePath);
            if (!playerDir.exists()) {
                players[playerId]->addMedia(QStringList());
            }
            else{
                QStringList fileNames = playerDir.entryList(QDir::Files);
                QStringList fullPaths;
                for (const QString &musicFileName : fileNames) {
                    fullPaths.append(playerDir.absoluteFilePath(musicFileName));
                }
                players[playerId]->addMedia(fullPaths);
            }
        }
        configFile.close();
    }
}

/**
 * @brief Loads the application settings from persistent storage and applies these settings.
 *
 * This function retrieves and sets various application-wide settings using `QSettings` and ensures
 * that the working directory, audio configurations, language, initiative tracker options, and
 * appearance themes are properly initialized.
 *
 * The settings are loaded as follows:
 *
 * - **General Settings**:
 *   - Loads the working directory path. If the directory does not exist, it is created.
 *
 * - **Music Settings**:
 *   - Sets the audio output device for all QPlayer instances.
 *   - Configures the volume slider to the saved volume level.
 *
 * - **Language Settings**:
 *   - Sets the application language by calling `changeLanguage`.
 *
 * - **Initiative Tracker Settings**:
 *   - Sets the health bar display mode.
 *   - Configures the visibility of specific fields in the initiative tracker widget.
 *
 * - **Appearance Settings**:
 *   - Loads the theme (Light, Dark, or a custom theme from XML) and applies it using `ThemeManager`.
 *
 * @details
 * If a setting is not found in persistent storage, a default value is used.
 *
 * @note The function assumes that all required objects (e.g., `players`, `ui`, `initiativeTrackerWidget`)
 * are properly initialized before this function is called.
 */
void MainWindow::loadSettings() {
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
    /// General
    workingDir = settings.value(paths.general.dir, workingDir).toString();
    QDir dir(workingDir);
    if (!dir.exists())
        dir.mkpath(".");
    defaultCampaignDir = settings.value(paths.general.defaultCampaignDir, "").toString();
    /// Music
    for (MusicPlayerWidget *player : players) {
        player->setAudioOutput(settings.value(paths.general.audioDevice, 0).toInt());
    }
    ui->volumeSlider->setValue(settings.value(paths.general.volume, 100).toInt());
    ///Language
    changeLanguage(settings.value(paths.general.lang, "ru_RU").toString());
    /// Initiative tracker
    initiativeTrackerWidget->setHpDisplayMode(settings.value(paths.initiative.hpBarMode, 0).toInt());
    autoRoll = settings.value(paths.initiative.autoInitiative, false).toBool();
    int initiativeFields = settings.value(paths.initiative.fields, 7).toInt();
    initiativeTrackerWidget->setSharedFieldVisible(0, initiativeFields & iniFields::name);
    initiativeTrackerWidget->setSharedFieldVisible(1, initiativeFields & iniFields::init);
    initiativeTrackerWidget->setSharedFieldVisible(2, initiativeFields & iniFields::ac);
    initiativeTrackerWidget->setSharedFieldVisible(3, initiativeFields & iniFields::hp);
    initiativeTrackerWidget->setSharedFieldVisible(4, initiativeFields & iniFields::maxHp);
    initiativeTrackerWidget->setSharedFieldVisible(5, initiativeFields & iniFields::del);

    /// Appearance
    QString theme = settings.value(paths.appearance.theme, "Light").toString();
    if (theme == "Light")
        ThemeManager::applyPreset(ThemeManager::PresetTheme::Light);
    else if (theme == "Dark")
        ThemeManager::applyPreset(ThemeManager::PresetTheme::Dark);
    else if (theme == "System")
        ThemeManager::resetToSystemTheme();
    else
        ThemeManager::loadFromXml(theme);

    QString style = settings.value(paths.appearance.style, "Fusion").toString();
    QApplication::setStyle(QStyleFactory::create(style));

    /// Campaign
    currentCampaignDir = settings.value(paths.session.campaign, "").toString();
    setupCampaign(currentCampaignDir);

    /// Rolls
    rollWidget->setCompactMode(settings.value(paths.rolls.compactMode).toBool());
}

/**
 * @brief Creates a new campaign project and initializes its directory structure and configuration.
 *
 * This function is responsible for setting up a new campaign by:
 * 1. Displaying a dialog (SaveConfigDialog) for the user to input the project name and directory.
 * 2. Validating the user's input and creating necessary subdirectories within the specified root.
 * 3. Generating a configuration file named "campaign.json" with the project details.
 * 4. Setting up the campaign environment using the specified directory.
 *
 * Behavior:
 * - Prompts the user with a SaveConfigDialog to specify the directory and project name.
 * - If the user cancels the dialog or provides invalid input, the function returns without making changes.
 * - Validates the directory path and attempts to create the following subdirectories: "Characters", "Maps", "Encounters", and "Music".
 *   If any subdirectory creation fails, the function displays a warning message and terminates the operation.
 * - Writes the initial project configuration in JSON format to the "campaign.json" file. If the file
 *   cannot be created or written to, a critical error message is displayed, and the operation is halted.
 * - Calls `setupCampaign` with the specified root directory to initialize the campaign environment and UI.
 *
 * Error Handling:
 * - Displays a warning message if it fails to create any subdirectory.
 * - Displays a critical error message if it cannot open or write to the configuration file.
 *
 * Requirements:
 * - The `SaveConfigDialog` object must be properly instantiated to collect user input.
 * - The directory path provided by the user must be valid and allow write operations.
 *
 * Dependencies:
 * - Requires `setupCampaign` to be implemented for initializing the campaign environment.
 *
 * Side Effects:
 * - Modifies the filesystem by creating new directories and files under the specified path.
 * - Updates the application's state to reflect the new campaign configuration.
 */
void MainWindow::newCampaign() {
    SaveConfigDialog dialog(this, defaultCampaignDir);
    QString fileName = "";

    if (dialog.exec() == QDialog::Accepted)
        fileName = dialog.filename;

    if (fileName.isEmpty()) return;

    QDir dir(dialog.directoryPath);

    QStringList subdirs = {"Characters", "Maps", "Encounters", "Music"};
    for (const QString& sub : subdirs){
        if (!dir.mkpath(sub)){
            QMessageBox::warning(this, "Error", tr("Can't create subdirectory: ") + sub);
            return;
        }
    }

    fileName = dir.filePath("campaign.json");

    QFile configFile(fileName);
    if (!configFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Open file error"), configFile.errorString());
        return;
    }

    QJsonObject obj;
    obj["name"] = dialog.projectName;

    QJsonDocument doc(obj);
    configFile.write(doc.toJson(QJsonDocument::Indented));
    configFile.close();

    dir.cdUp();
    defaultCampaignDir = dir.absolutePath();

    setupCampaign(dialog.directoryPath);
}


/**
 * @brief Handles the "Settings" action when triggered.
 *
 * This function is invoked when the user selects the "Settings" option from the UI.
 * It performs the following operations:
 *
 * 1. Saves the current application settings by calling `saveSettings`.
 * 2. Checks if the settings dialog (`settingsDialog`) exists. If not, it initializes
 *    the dialog by creating a new `SettingsDialog` object with the organization name,
 *    application name, and parent as parameters.
 * 3. Displays the settings dialog in a modal state by calling `exec` on it.
 * 4. After the dialog is closed, it reloads application settings by calling `loadSettings`.
 *
 * @note
 * - The `settingsDialog` is initialized only once and reused afterward.
 * - The `saveSettings` function is called before showing the dialog to ensure
 *   the latest settings are preserved.
 * - The `loadSettings` function is called after the dialog is closed, allowing
 *   the application to reflect any changes made in the settings dialog.
 */
void MainWindow::on_actionSettings_triggered() {
    saveSettings();
    if(!settingsDialog)
    {
        settingsDialog = new SettingsDialog(ORGANIZATION_NAME, APPLICATION_NAME, this);
    }
    settingsDialog->exec();
    loadSettings();
}


/**
 * @brief Opens and loads a map file into the application.
 *
 * This method is responsible for loading a map file into a new tab in the `mapTabWidget`.
 * Based on the file extension, it either loads a scene (for "dam" files) or displays a map
 * image directly. If the operation is successful, a new `MapView` instance is created and
 * added as a tab with the file's base name as the tab title. The visibility of the user
 * interface is updated accordingly, and the new tab becomes the active tab.
 * Additionally, this method connects the `toolChanged` signal of the `MapScene` to the
 * `toolGroup` actions to manage their state appropriately.
 *
 * If the file fails to load, the method displays an error dialog and ensures that the
 * `MapView` instance created earlier is properly deleted to avoid memory leaks.
 *
 * @param fileName The path to the map file to load.
 */
void MainWindow::openMapFromFile(const QString& fileName) {
    QFileInfo fileInfo(fileName);
    QString ext = fileInfo.suffix().toLower();

    auto *view = new MapView(this);
    bool success;

    if (ext == "dam") {
        success = view->loadSceneFromFile(fileName);
    } else {
        view->loadMapImage(fileName);
        success = true;
    }

    if (success) {
        mapTabWidget->addTab(view, fileInfo.baseName());
        updateVisibility();
        mapTabWidget->setCurrentIndex(mapTabWidget->count() - 1);

        connect(view->getScene(), &MapScene::toolChanged, this, [=](const AbstractMapTool* tool){
            if (!tool){
                for (QAction *action : toolGroup->actions()) {
                    action->setChecked(false);
                }
            }
        });
    } else {
        delete view;
        QMessageBox::warning(this, tr("Error"), tr("Failed to open map file."));
    }
}

/**
 * @brief Opens or activates a shared map window for the specified tab index.
 *
 * This method manages a `SharedMapWindow` instance to display the map from
 * a specific tab in a separate window. If a shared map window does not already
 * exist, it initializes a new `SharedMapWindow` instance with the map scene
 * from the specified tab, displays it, and manages its lifecycle. If a shared
 * map window already exists, it brings the existing window to the foreground.
 *
 * @param index Index of the current map tab to get the corresponding map scene.
 */
void MainWindow::openSharedMapWindow(int index) {
    auto* currentView = qobject_cast<MapView*>(mapTabWidget->widget(index));
    if (!sharedMapWindow){
        sharedMapWindow = new SharedMapWindow(currentView->getScene());
        sharedMapWindow->show();
        sharedMapWindow->resize(800, 600);

        connect(sharedMapWindow, &QWidget::destroyed, this, [=]() {
            sharedMapWindow = nullptr;
        });
    } else{
        sharedMapWindow->raise();
        sharedMapWindow->activateWindow();
    }
}

/**
 * @brief Saves the current campaign data, including music configuration and maps.
 *
 * This function performs the following actions in sequence:
 * 1. Retrieves the root path from the campaign tree widget.
 * 2. If the root path is invalid or the directory does not exist, the function returns without making any changes.
 * 3. Removes the "Music" directory (without deleting the base directory itself) and regenerates the music configuration file
 *    at the path `rootPath + "/Music/playerConfig.xml"`.
 * 4. Iterates over all map tabs in `mapTabWidget` and saves each map as a .dam file in the `Maps` subdirectory
 *    within the root campaign directory.
 *
 * Preconditions:
 * - The `campaignTreeWidget` should be initialized and its `root()` method must return a valid path for the campaign root directory.
 * - The `mapTabWidget` should contain child widgets for each map to be exported.
 *
 * Postconditions:
 * - The "Music" directory within the campaign root may be emptied and its configuration file regenerated.
 * - Each map is exported into the appropriate directory based on its tab index and object name.
 */
void MainWindow::closeCampaign() {
    QString rootPath = campaignTreeWidget->root();
    QDir rootDir(rootPath);
    if (rootPath.isEmpty() || !rootDir.exists())
        return;

    removeDirectoryRecursively(rootPath + "/Music", false);
    saveMusicConfigFile(rootPath + "/Music/playerConfig.xml");

    for (int i = 0; i < mapTabWidget->count(); ++i) {
        exportMap(rootPath + "/Maps/" + mapTabWidget->widget(i)->objectName(), i);
    }

    currentCampaignDir = "";
    campaignTreeWidget->clear();
    initiativeTrackerWidget->setBaseDir();
}

/**
 * Saves the current configuration of the application to an XML file.
 * This function allows the user to specify a filename using a dialog,
 * and then writes the configuration of playlists and their associated paths
 * for all players into an XML structure in the specified file.
 *
 * - If the user cancels or does not specify a filename, the function will exit early.
 * - If the file cannot be opened for read/write operations, a critical error message
 *   is displayed and the function returns.
 * - Each player's playlist and associated local data are moved to a directory
 *   corresponding to the new configuration base directory.
 *
 * XML Structure Example:
 * The resulting XML structure for playlists will look like:
 * <music-player>
 *     <playlist id="0">path/to/playlist0</playlist>
 *     <playlist id="1">path/to/playlist1</playlist>
 *     ...
 *     <playlist id="8">path/to/playlist8</playlist>
 * </music-player>
 *
 * Internally, it uses:
 * - QDomDocument to create the XML document.
 * - @ref moveAllFiles to move the files from the local directory of each playlist
 *   to the path corresponding to the new configuration.
 *
 * Preconditions:
 * - The QVector<QPlayer*> `players` should be initialized and populated with
 *   all player instances before invoking this function.
 *
 * Postconditions:
 * - A new XML configuration file is created at the specified location.
 * - Local directories of all playlists are moved to the base directory.
 */
void MainWindow::saveMusicConfigFile(const QString& fileName) {
    if(fileName.isEmpty())
        return;

    QFile configFile(fileName);
    if (!configFile.open(QIODevice::ReadWrite))
    {
        QMessageBox::critical(this, tr("Open file error"), configFile.errorString());
        return;
    }

    QFileInfo fileInfo(configFile.fileName());
    QString baseDir = fileInfo.canonicalPath() + "/";

    QTextStream xmlContent(&configFile);
    QDomDocument configDocument;

    QDomElement root = configDocument.createElement("music-player");
    configDocument.appendChild(root);

    for (int i = 0; i < 9; ++i) {
        QDomElement playlist_node = configDocument.createElement("playlist");
        playlist_node.setAttribute("id", i);
        playlist_node.setAttribute("volume", players[i]->volumeSliderPosition());

        QString playlistPath = baseDir + players[i]->getPlaylistName();
        moveAllFiles(players[i]->getLocalDirPath(), playlistPath);

        QDomText textNode = configDocument.createTextNode(playlistPath);
        playlist_node.appendChild(textNode);

        root.appendChild(playlist_node);
    }

    xmlContent << configDocument.toString();
    configFile.close();
}

/**
 * @brief Saves application settings to persistent storage.
 *
 * This function stores the current working directory and the volume slider's
 * value in QSettings under the keys defined by the `paths.general.dir` and
 * `paths.general.volume` respectively. After setting the values, it synchronizes
 * the settings to ensure the data is saved properly.
 *
 * The organization name and application name used for QSettings are defined by
 * the constants `ORGANIZATION_NAME` and `APPLICATION_NAME`.
 */
void MainWindow::saveSettings() {
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
    settings.setValue(paths.general.dir, workingDir);
    settings.setValue(paths.general.volume, ui->volumeSlider->value());
    settings.setValue(paths.general.defaultCampaignDir, defaultCampaignDir);
    settings.setValue(paths.session.campaign, campaignTreeWidget->root());
    settings.setValue(paths.rolls.compactMode, rollWidget->compactMode());
    settings.sync();
}

/**
 * @brief Configures and sets up the campaign interface and related components.
 *
 * This function initializes the campaign environment based on the provided campaign directory.
 * It configures the campaign tree widget, connects its signals to appropriate slots for handling
 * encounters and maps, and updates the user interface by adding the campaign widget to the layout.
 * Additionally, it handles the loading or creation of the music configuration file for player playlists.
 *
 * Behavior:
 * - Checks if the provided campaign directory path is valid and non-empty before proceeding.
 * - Attempts to set the root directory of the CampaignTreeWidget. If unsuccessful, the function exits.
 * - Connects CampaignTreeWidget signals to relevant slots for handling encounter addition, encounter
 *   replacement, and map opening requests.
 * - Adds the CampaignTreeWidget to the campaign layout in the user interface and makes it visible.
 * - Constructs the path to the music configuration file within the campaign directory. If the file exists,
 *   it loads the configuration using `loadMusicConfigFile`. Otherwise, a new configuration file is saved
 *   using `saveMusicConfigFile`.
 * - Updates the current active campaign directory (`currentCampaignDir`) to the provided path.
 *
 * @param campaignRoot The root directory of the campaign to be set up.
 *                     It should be a valid and accessible directory path.
 */
void MainWindow::setupCampaign(const QString campaignRoot) {
    if (campaignRoot.isEmpty())
        return;

    if (!currentCampaignDir.isEmpty()){
        closeCampaign();
    }

    if (!campaignTreeWidget->setRootDir(campaignRoot))
        return;

    foreach(MusicPlayerWidget* player, players){
        removeDirectoryRecursively(player->getLocalDirPath());
    }

    QString musicConfigFile = campaignRoot + "/Music/playerConfig.xml";
    if (QFile(musicConfigFile).exists())
        loadMusicConfigFile(musicConfigFile);
    else
        saveMusicConfigFile(musicConfigFile);

    currentCampaignDir = campaignRoot;
    initiativeTrackerWidget->setBaseDir(currentCampaignDir + "/Encounters");
    campaignTreeWidget->setVisible(true);
}

/**
 * Initializes and configures player widgets in the MainWindow.
 *
 * This function dynamically creates nine QPlayer objects with unique identifiers
 * and titles. These QPlayer instances are added to the `players` QVector and displayed
 * in the UI layout defined as `ui->musicLayout`. Additionally, signal-slot connections
 * are established to ensure that when a player starts, all other players stop.
 *
 * The `QPlayer` instances are assigned sequential IDs from 1 to 9 and titled
 * "Player 1" through "Player 9". Each player is inserted into the layout at
 * the appropriate position and connected to the `stopOtherPlayers` slot to
 * handle stopping other players when one starts.
 */
void MainWindow::setupPlayers() {
    for (int i = 0; i < 9; ++i) {
        auto *player = new MusicPlayerWidget(this, i + 1, QString("Player %1").arg(i + 1));
        connect(this, &MainWindow::translatorChanged, player, &MusicPlayerWidget::updateTranslator);
        players.append(player);
    }

    for (int i = 0; i < players.size(); ++i) {
        ui->musicLayout->insertWidget(i, players[i]);
        connect(players[i], SIGNAL(playerStarted(int)), this, SLOT(stopOtherPlayers(int)));
    }
}

/**
 * @brief Configures and initializes the map-related components within the main window.
 *
 * This method sets up the `mapTabWidget` for managing multiple map tabs, configures the layout and alignment of map-related
 * widgets, and establishes connections between UI elements and their respective actions. It also ensures the visibility of
 * the map widgets is updated based on their state.
 *
 * The following actions and connections are initialized:
 * - Creates the `mapTabWidget` and adds it to the main layout.
 * - Aligns placeholder layout to the center.
 * - Connects the `openMapButton` to trigger the creation of a new map tab.
 * - Connects the `newTabRequested` signal of `mapTabWidget` to invoke the creation of a new map tab.
 * - Connects the `AddMap` QAction to trigger new map tab creation.
 * - Links the `tabCloseRequested` signal of `mapTabWidget` to handle closing and deleting tabs.
 * - Connects the `share` signal of `mapTabWidget` to open the shared map window.
 * - Connects the `save` signal of `mapTabWidget` to perform map export.
 *
 * After setting up these connections and components, it ensures the visibility
 * of the placeholder and the map tabs is updated accordingly.
 */
void MainWindow::setupMaps() {
    mapTabWidget = new TabWidget(this);
    ui->mainViewLayout->addWidget(mapTabWidget);
    ui->phLayout->setAlignment(Qt::AlignCenter);
    connect(ui->openMapButton, &QPushButton::clicked, this, &MainWindow::createNewMapTab);
    connect(mapTabWidget, &TabWidget::newTabRequested, this, &MainWindow::createNewMapTab);
    connect(ui->actionAddMap, &QAction::triggered, this, &MainWindow::createNewMapTab);
    connect(mapTabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::deleteMapTab);
    connect(mapTabWidget, &TabWidget::share, this, &MainWindow::openSharedMapWindow);
    connect(mapTabWidget, &TabWidget::save, this, &MainWindow::slotExportMap);
    connect(mapTabWidget, &TabWidget::dropAccepted, this, &MainWindow::openMapFromFile);
    updateVisibility();
}

/**
 * @brief Sets up keyboard shortcuts for player controls.
 *
 * This function iterates through a collection of players and assigns
 * a unique keyboard shortcut to each player. The shortcuts are
 * generated in the form "Ctrl+<index>", where `<index>` corresponds
 * to the player's index in the `players` vector. These shortcuts
 * enable quick access to the play functionality of each player.
 *
 * Preconditions:
 * - The `players` vector must be initialized and contain valid player objects.
 * - Each player in the `players` vector must support the `setPlayShortcut` method.
 */
void MainWindow::setupShortcuts() {
    for (int i = 0; i < players.size(); ++i) {
        QString key = QString("Ctrl+%1").arg(i + 1);
        players[i]->setPlayShortcut(key);
    }
}

/**
 * @brief Configures the toolbar of the main window.
 *
 * This function initializes tool widgets and action groups, adds tools for various
 * functionalities, and connects the respective signals and slots for interaction.
 * Tools include rulers, fog tools, light tools, and shape drawing tools such as lines,
 * circles, and squares. Context menus are also set for specific tools, along with
 * tooltips and icon configurations.
 *
 * Key functionalities implemented in the toolbar setup:
 *   - Ruler Tool: Allows calibration and measurement mode toggling. Context menu provides
 *     calibration options.
 *   - Fog Tools: Includes fog coverage tools (hide/reveal) with context menu actions for
 *     bulk fog manipulation (e.g., hiding or revealing all fog).
 *   - Light Tool: Provides controls for adding light sources to the map. Includes options
 *     for modifying light radius and color and an additional "update fog" checkbox for
 *     automatic fog updates.
 *   - Shape Drawing Tools: Enables drawing of line, circle, and square shapes on the map,
 *     with each shape having its own tool button and connectivity to the current map view.
 *   - Brush tool: Provides controls for drawing brush strokes on the map.
 *
 * Actions and widgets added to the toolbar are connected dynamically to their underlying
 * tools and modes, ensuring that the toolbar functions are seamlessly integrated with
 * user interactions.
 */
void MainWindow::setupToolbar() {
    ui->toolBar->setMovable(false);

    brushTool = new BrushTool(this);
    calibrationTool = new CalibrationTool(this);
    fogTool = new FogTool(this);
    lightTool = new LightTool(this);
    rulerMapTool = new RulerTool(this);
    lineShapeTool = new LineShapeTool();
    circleShapeTool = new CircleShapeTool();
    squareShapeTool = new SquareShapeTool();
    triangleShapeTool = new TriangleShapeTool();
    heightMapTool = new HeightMapTool();

    toolGroup = new QActionGroup(this);
    toolGroup->setExclusive(true);

    /// Ruler tool
    auto *rulerButton = new QToolButton(this);
    rulerButton->setCheckable(true);
    rulerButton->setToolTip(tr("ruler"));

    auto *rulerAction = new QAction(this);
    rulerAction->setCheckable(true);
    rulerAction->setChecked(false);
    toolGroup->addAction(rulerAction);
    ThemedIconManager::instance().addIconTarget(":/map/ruler.svg", rulerAction, &QAction::setIcon);
    rulerButton->setDefaultAction(rulerAction);

    rulerButton->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(rulerButton, &QToolButton::customContextMenuRequested, this, [=](const QPoint &pos) {
        QMenu contextMenu;
        QAction *calibrateAct = contextMenu.addAction(tr("calibrate"));
        QAction *chosen = contextMenu.exec(rulerButton->mapToGlobal(pos));
        if (chosen == calibrateAct) {
            // временно активируем CalibrationTool
            auto* currentView = qobject_cast<MapView*>(mapTabWidget->currentWidget());
            if (!currentView) return;
                currentView->setActiveTool(calibrationTool);
            // отключим кнопку линейки на время
            rulerAction->setChecked(false);
        }
    });
    connect(rulerAction, &QAction::triggered, [=](bool checked){
        auto* currentView = qobject_cast<MapView*>(mapTabWidget->currentWidget());
        if (!currentView) return;
        if (checked)
            currentView->setActiveTool(rulerMapTool);
        else
            currentView->setActiveTool(nullptr);
    });
    connect(calibrationTool, &AbstractMapTool::finished, [=]() {
        auto* currentView = qobject_cast<MapView*>(mapTabWidget->currentWidget());
        if (currentView)
            currentView->setActiveTool(rulerMapTool);
        rulerAction->setChecked(true);
    });
    ui->toolBar->addWidget(rulerButton);

    /// Fog-hide tool
    auto* fogHideAction = new QAction(this);
    fogHideAction->setCheckable(true);
    ThemedIconManager::instance().addIconTarget(":/map/fog_hide.svg", fogHideAction, &QAction::setIcon);
    toolGroup->addAction(fogHideAction);

    auto* fogHideButton = new QToolButton(this);
    fogHideButton->setCheckable(true);
    fogHideButton->setToolTip(tr("Add fog to map"));
    fogHideButton->setDefaultAction(fogHideAction);
    ui->toolBar->addWidget(fogHideButton);

    fogHideButton->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(fogHideButton, &QToolButton::customContextMenuRequested, this, [=](const QPoint pos){
        QMenu contextMenu;
        QAction* hideAllAction = contextMenu.addAction(tr("Hide all"));
        QAction* chosen = contextMenu.exec(fogHideButton->mapToGlobal(pos));
        if (chosen == hideAllAction)
            coverMapWithFog(true);
    });


    connect(fogHideAction, &QAction::triggered, this, [=](bool checked){
        auto* currentView = qobject_cast<MapView*>(mapTabWidget->currentWidget());
        if (!currentView) return;
        if (!checked){
            currentView->setActiveTool(nullptr);
        } else{
            fogTool->setMode(FogTool::Hide);
            currentView->setActiveTool(fogTool);
        }
    });

    /// Fog-reveal tool
    auto* fogRevealAction = new QAction(this);
    fogRevealAction->setCheckable(true);
    ThemedIconManager::instance().addIconTarget(":/map/fog_reveal.svg", fogRevealAction, &QAction::setIcon);
    toolGroup->addAction(fogRevealAction);

    auto* fogRevealButton = new QToolButton(this);
    fogRevealButton->setCheckable(true);
    fogRevealButton->setToolTip(tr("Remove fog from map"));
    fogRevealButton->setDefaultAction(fogRevealAction);
    ui->toolBar->addWidget(fogRevealButton);

    fogRevealButton->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(fogRevealButton, &QToolButton::customContextMenuRequested, this, [=](const QPoint pos){
        QMenu contextMenu;
        QAction* revealAllAction = contextMenu.addAction(tr("Reveal all"));
        QAction* chosen = contextMenu.exec(fogHideButton->mapToGlobal(pos));
        if (chosen == revealAllAction)
            coverMapWithFog(false);
    });

    connect(fogRevealAction, &QAction::triggered, this, [=](bool checked){
        auto* currentView = qobject_cast<MapView*>(mapTabWidget->currentWidget());
        if (!currentView) return;
        if (!checked){
            currentView->setActiveTool(nullptr);
        } else{
            fogTool->setMode(FogTool::Reveal);
            currentView->setActiveTool(fogTool);
        }
    });

    ui->toolBar->addSeparator();

    /// Light tool
    auto* lightAction = new QAction(this);
    lightAction->setCheckable(true);
    ThemedIconManager::instance().addIconTarget(":/map/torch.svg", lightAction, &QAction::setIcon);
    toolGroup->addAction(lightAction);

    auto* lightButton = new QToolButton(this);
    lightButton->setCheckable(true);
    lightButton->setToolTip(tr("Edit light sources"));
    lightButton->setDefaultAction(lightAction);
    ui->toolBar->addWidget(lightButton);

    auto *brightRadiusBox = new QSpinBox;
    brightRadiusBox->setToolTip(tr("Bright radius"));
    brightRadiusBox->setRange(1, 500);
    brightRadiusBox->setValue(20);
    lightTool->setBrightRadius(brightRadiusBox->value());
    ui->toolBar->addWidget(brightRadiusBox);

    auto *dimRadiusBox = new QSpinBox;
    dimRadiusBox->setToolTip(tr("Dim radius"));
    dimRadiusBox->setRange(1, 1000);
    dimRadiusBox->setValue(40);
    lightTool->setDimRadius(dimRadiusBox->value());
    ui->toolBar->addWidget(dimRadiusBox);

    auto *lightColorBtn = new QPushButton();
    ThemedIconManager::instance().addIconTarget<QAbstractButton>(":/map/palette.svg", lightColorBtn, &QAbstractButton::setIcon);
    ui->toolBar->addWidget(lightColorBtn);

    connect(lightAction, &QAction::triggered, [=](bool checked){
        auto* currentView = qobject_cast<MapView*>(mapTabWidget->currentWidget());
        if (!currentView) return;
        if (checked)
            currentView->setActiveTool(lightTool);
        else
            currentView->setActiveTool(nullptr);
    });

    connect(brightRadiusBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int value){
        dimRadiusBox->setMinimum(value);
        lightTool->setBrightRadius(value);
    });

    connect(dimRadiusBox, QOverload<int>::of(&QSpinBox::valueChanged),
            lightTool, &LightTool::setDimRadius);

    connect(lightColorBtn, &QPushButton::clicked, this, [=]() {
        QColor chosen = QColorDialog::getColor(lightTool->color(), this);
        if (chosen.isValid()) {
            lightTool->setColor(chosen);
        }
    });

    auto *fogUpdateBox = new QCheckBox(tr("Update fog"));
    fogUpdateBox->setChecked(false);
    ui->toolBar->addWidget(fogUpdateBox);

    connect(fogUpdateBox, &QCheckBox::toggled, lightTool, &LightTool::setAutoUpdateFog);

    ui->toolBar->addSeparator();

    /// Spells
    /// LineShapeTool
    auto* lineAction = new QAction(this);
    lineAction->setCheckable(true);
    ThemedIconManager::instance().addIconTarget(":/map/line.svg", lineAction, &QAction::setIcon);
    toolGroup->addAction(lineAction);

    auto* lineButton = new QToolButton(this);
    lineButton->setCheckable(true);
    lineButton->setToolTip(tr("Draw line"));
    lineButton->setDefaultAction(lineAction);
    ui->toolBar->addWidget(lineButton);

    connect(lineAction, &QAction::triggered, this, [=](bool checked){
        auto* currentView = qobject_cast<MapView*>(mapTabWidget->currentWidget());
        if (!currentView) return;
        if (checked)
            currentView->setActiveTool(lineShapeTool);
        else
            currentView->setActiveTool(nullptr);
    });

    /// CircleShapeTool
    auto* circleAction = new QAction(this);
    circleAction->setCheckable(true);
    ThemedIconManager::instance().addIconTarget(":/map/sphere.svg", circleAction, &QAction::setIcon);
    toolGroup->addAction(circleAction);

    auto* circleButton = new QToolButton(this);
    circleButton->setCheckable(true);
    circleButton->setToolTip(tr("Draw circle"));
    circleButton->setDefaultAction(circleAction);
    ui->toolBar->addWidget(circleButton);

    connect(circleAction, &QAction::triggered, this, [=](bool checked){
        auto* currentView = qobject_cast<MapView*>(mapTabWidget->currentWidget());
        if (!currentView) return;
        if (checked)
            currentView->setActiveTool(circleShapeTool);
        else
            currentView->setActiveTool(nullptr);
    });

    /// SquareShapeTool
    auto* squareAction = new QAction(this);
    squareAction->setCheckable(true);
    ThemedIconManager::instance().addIconTarget(":/map/cube.svg", squareAction, &QAction::setIcon);
    toolGroup->addAction(squareAction);

    auto* squareButton = new QToolButton(this);
    squareButton->setCheckable(true);
    squareButton->setToolTip(tr("Draw square"));
    squareButton->setDefaultAction(squareAction);
    ui->toolBar->addWidget(squareButton);

    connect(squareAction, &QAction::triggered, this, [=](bool checked){
        auto* currentView = qobject_cast<MapView*>(mapTabWidget->currentWidget());
        if (!currentView) return;
        if (checked)
            currentView->setActiveTool(squareShapeTool);
        else
            currentView->setActiveTool(nullptr);
    });

    /// TriangleShapeTool
    auto* triangleAction = new QAction(this);
    triangleAction->setCheckable(true);
    ThemedIconManager::instance().addIconTarget(":/map/cone.svg", triangleAction, &QAction::setIcon);
    toolGroup->addAction(triangleAction);

    auto* triangleButton = new QToolButton(this);
    triangleButton->setCheckable(true);
    triangleButton->setToolTip(tr("Draw triangle"));
    triangleButton->setDefaultAction(triangleAction);
    ui->toolBar->addWidget(triangleButton);

    connect(triangleAction, &QAction::triggered, this, [=](bool checked){
        auto* currentView = qobject_cast<MapView*>(mapTabWidget->currentWidget());
        if (!currentView) return;
        if (checked)
            currentView->setActiveTool(triangleShapeTool);
        else
            currentView->setActiveTool(nullptr);
    });


    /// Brush tool
    auto* brushAction = new QAction(this);
    brushAction->setCheckable(true);
    ThemedIconManager::instance().addIconTarget(":/map/brush.svg", brushAction, &QAction::setIcon);
    toolGroup->addAction(brushAction);

    auto* brushButton = new QToolButton(this);
    brushButton->setCheckable(true);
    brushButton->setToolTip(tr("Brush"));
    brushButton->setDefaultAction(brushAction);
    ui->toolBar->addWidget(brushButton);

    brushButton->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(brushButton, &QToolButton::customContextMenuRequested, this, [=](const QPoint pos) {
        QMenu contextMenu;
        QAction* clearAllAction = contextMenu.addAction(tr("Clear all"));
        QAction* chosen = contextMenu.exec(brushButton->mapToGlobal(pos));
        if (chosen == clearAllAction){
            auto* currentView = qobject_cast<MapView*>(mapTabWidget->currentWidget());
            brushTool->clearAll(currentView->getScene());
        }
    });

    connect(brushAction, &QAction::triggered, this, [=](bool checked){
        auto* currentView = qobject_cast<MapView*>(mapTabWidget->currentWidget());
        if (!currentView) return;
        if (checked)
            currentView->setActiveTool(brushTool);
        else
            currentView->setActiveTool(nullptr);
    });


    /// Brush opacity
    auto* opacitySlider = new QSlider(Qt::Horizontal);
    opacitySlider->setRange(0, 100);
    opacitySlider->setValue(50);
    opacitySlider->setToolTip(tr("Brush opacity"));
    opacitySlider->setMaximumWidth(80);
    ui->toolBar->addWidget(opacitySlider);

    connect(opacitySlider, &QSlider::valueChanged, this, [=](int v) {
        brushTool->setOpacity(v / 100.0);
    });

    /// Shape color button
    auto *ShapeToolColorButton = new QPushButton();
    ThemedIconManager::instance().addIconTarget<QAbstractButton>(":/map/palette.svg", ShapeToolColorButton, &QAbstractButton::setIcon);
    ui->toolBar->addWidget(ShapeToolColorButton);

    connect(ShapeToolColorButton, &QPushButton::clicked, this, [=]() {
        QColor chosen = QColorDialog::getColor(lightTool->color(), this);
        if (chosen.isValid()) {
            lineShapeTool->setColor(chosen);
            circleShapeTool->setColor(chosen);
            squareShapeTool->setColor(chosen);
            triangleShapeTool->setColor(chosen);
            brushTool->setColor(chosen);
        }
    });


    ui->toolBar->addSeparator();
    /// Height Map
    auto* heightMapAction = new QAction(this);
    heightMapAction->setCheckable(true);
    ThemedIconManager::instance().addIconTarget(":map/mountain.svg", heightMapAction, &QAction::setIcon);
    toolGroup->addAction(heightMapAction);

    auto* heightButton = new QToolButton(this);
    heightButton->setCheckable(true);
    heightButton->setToolTip(tr("Height"));
    heightButton->setDefaultAction(heightMapAction);
    ui->toolBar->addWidget(heightButton);

    connect(heightMapAction, &QAction::triggered, [=](bool checked){
        auto* currentView = qobject_cast<MapView*>(mapTabWidget->currentWidget());
        if (!currentView) return;
        if (checked)
            currentView->setActiveTool(heightMapTool);
        else
            currentView->setActiveTool(nullptr);
    });
}

/**
 * @brief Sets up the initiative tracker widget in the main window.
 *
 * This function initializes a new instance of InitiativeTrackerWidget
 * and adds it to the tracker layout of the main window's UI. The
 * widget is created with the main window as its parent.
 */
void MainWindow::setupTracker() {
    initiativeTrackerWidget = new InitiativeTrackerWidget(this);
    ui->trackerLayout->addWidget(initiativeTrackerWidget);

    connect(campaignTreeWidget, &CampaignTreeWidget::encounterReplaceRequested, initiativeTrackerWidget, &InitiativeTrackerWidget::loadFromFile);
    connect(campaignTreeWidget, &CampaignTreeWidget::encounterAddRequested, initiativeTrackerWidget, &InitiativeTrackerWidget::addFromFile);
    connect(campaignTreeWidget, &CampaignTreeWidget::encounterReplaceRequested, initiativeTrackerWidget, &InitiativeTrackerWidget::loadFromFile);
    connect(this, &MainWindow::translatorChanged, initiativeTrackerWidget, &InitiativeTrackerWidget::updateTranslator);
}

/**
 * @brief Sets the volume divider for all QPlayer instances managed by MainWindow.
 *
 * This function iterates through the `players` vector and sets the volume divider
 * for each QPlayer object to the specified value. The operation assumes that the
 * `players` vector contains exactly 9 QPlayer instances.
 *
 * @param value The volume divider value to be applied to each QPlayer instance.
 */
void MainWindow::setVolumeDivider(int value) {
    for (int i = 0; i < 9; ++i) {
        players[i]->setVolumeDivider(value);
    }
}

/**
 * @brief Stops all player instances in the application.
 *
 * Iterates through all QPlayer objects in the `players` vector and invokes
 * the `stop` method on each. This is typically used to halt any ongoing playback
 * or activities managed by the players.
 */
void MainWindow::stopAll() {
    for (int i = 0; i < 9; ++i) {
        players[i]->stop();
    }
}

/**
 * Stops all players in the `players` vector except the one specified by the given `exeptId`.
 *
 * Iterates over the list of players and checks the playlist ID of each player.
 * If the player's playlist ID does not match the provided `exeptId`, the player is stopped.
 *
 * @param exeptId The playlist ID of the player that should not be stopped.
 */
void MainWindow::stopOtherPlayers(int exeptId) {
    for (int i = 0; i < 9; ++i) {
        if (players[i]->getPlaylistId() != exeptId)
            players[i]->stop();
    }
}

/**
 * @brief Updates the visibility of specific UI elements based on the presence of tabs.
 *
 * This method checks whether the `mapTabWidget` contains any tabs. If tabs are present,
 * the `mapTabWidget` is made visible, and the placeholder widget (`ui->placeHolderWidget`)
 * is hidden. Otherwise, the `mapTabWidget` is hidden, and the placeholder widget is shown.
 *
 * The method ensures that appropriate UI elements are shown depending on whether there are
 * tabs to display in the `mapTabWidget`.
 */
void MainWindow::updateVisibility() {
    bool hasTabs = mapTabWidget->count() > 0;
    mapTabWidget->setVisible(hasTabs);
    ui->placeHolderWidget->setVisible(!hasTabs);
}

void MainWindow::addCharacter() {
    bool ok;
    QString characterName = QInputDialog::getText(this,
                                                  tr("Chose character name"),
                                                  tr("Character name:"),
                                                  QLineEdit::Normal,
                                                  "Nameless character",
                                                  &ok);
    if (!ok || characterName.isEmpty())
        return;

    if (currentCampaignDir.isEmpty()){
        QMessageBox::warning(this, tr("Can't add character"), tr("Open campaign first!"));
        return;
    }

    QDir dstDir(currentCampaignDir + "/Characters");
    QDir srcDir(qApp->applicationDirPath() + "/file-templates");

    QString srcFilePath = srcDir.filePath("dnd-character-template.json");
    QString dstFilePath = dstDir.filePath(characterName + ".json");

    if (QFile::exists(dstFilePath))
    {
        int ans = QMessageBox::warning(this, tr("Can't add character"), tr("Character already exists \n Override?"), QMessageBox::Yes | QMessageBox::Cancel);
        if (ans == QMessageBox::Cancel)
            return;
        else
            QFile::remove(dstFilePath);
    }

    QFile src(srcFilePath);

    if (!QFile::copy(srcFilePath, dstFilePath)){
        QMessageBox::warning(this, tr("Can't add character"), tr("Unknown error"));
        return;
    }

    setupCampaign(campaignTreeWidget->root());
    campaignTreeWidget->characterOpenRequested(dstFilePath);
}

void MainWindow::on_muteButton_clicked() {
    if (isMuted){
        isMuted = false;
        ui->volumeSlider->setValue(prevVolume);
        ThemedIconManager::instance().addIconTarget<QAbstractButton>(":/player/Volume-1.svg", ui->muteButton, &QAbstractButton::setIcon);
    } else{
        ThemedIconManager::instance().addIconTarget<QAbstractButton>(":/player/mute.svg", ui->muteButton, &QAbstractButton::setIcon);
        isMuted = true;
        prevVolume = ui->volumeSlider->value();
        ui->volumeSlider->setValue(0);
    }

}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event) {
    QStringList files;
    QString campaignPath = "";
    for (const QUrl &url : event->mimeData()->urls()) {
        QString path = url.toLocalFile();
        QFileInfo info(path);
        if (info.exists() && info.isFile() && (info.suffix().toLower() == "json")) {
            files.append(info.dir().absolutePath());
        }
    }

    if (files.size() == 1){
        campaignPath = files[0];

        QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                                  tr("Load campaign?"),
                                                                  tr("Load campaign from %1?").arg(campaignPath),
                                                                  QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes)
            setupCampaign(campaignPath);
    }
}


void MainWindow::showSourcesMessageBox(const QMap<QString, QString> &sources)
{
    QString html;
    html += "<html><body><ul>";

    for (auto it = sources.constBegin(); it != sources.constEnd(); ++it) {
    html += QString("<li><a href=\"%1\">%2</a></li>")
    .arg(it.value(), it.key());
    }

    html += "</ul></body></html>";

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Sources");
    msgBox.setText("List of used sources:");


    QTextBrowser *textBrowser = new QTextBrowser;
    textBrowser->setHtml(html);
    textBrowser->setOpenExternalLinks(true);

    textBrowser->setMinimumSize(400, 200);

    msgBox.layout()->addWidget(textBrowser);

    msgBox.exec();
}

void MainWindow::handleUpdates(bool hasUpdates) {
    if (hasUpdates){
        QString latest = updateChecker->latestVersion();
        QString latestUrl = updateChecker->latestUrl();
        ui->updateBanner->setCurrentVersion(VERSION);
        ui->updateBanner->setLatestVersion(latest);
        ui->updateBanner->setUrl(latestUrl);
        ui->updateBanner->show();
    }
}


/**
 * Copies all files from a source directory to a destination directory.
 * If the destination directory does not exist, it is created.
 * Existing files in the destination directory with the same name
 * as the source files are overwritten.
 *
 * @param sourcePath The path of the source directory containing the files to be copied.
 * @param destPath The path of the destination directory where the files will be copied to.
 */
static void copyAllFiles(const QString& sourcePath, const QString& destPath){
    QDir sourceDir(sourcePath);
    if (!sourceDir.exists())
        return;

    QDir destDir(destPath);
    if (!destDir.exists())
        destDir.mkpath(".");

    QStringList files = sourceDir.entryList(QDir::Files);
    foreach(const QString& file, files){
        QString srcFilePath = sourceDir.filePath(file);
        QString destFilePath = destDir.filePath(file);

        if (QFile::exists(destFilePath))
            QFile::remove(destFilePath);

        if (!QFile::copy(srcFilePath, destFilePath))
            return;
    }
}

/**
 * Moves all files from a source directory to a destination directory.
 * This function first copies all files from the source directory to the
 * destination directory and then removes the original files from the
 * source directory. If the destination directory does not exist, it is
 * created. Existing files in the destination directory with the same name
 * as the source files are overwritten.
 *
 * @param sourcePath The path of the source directory containing the files to be moved.
 * @param destPath The path of the destination directory where the files will be moved to.
 */
static void moveAllFiles(const QString& sourcePath, const QString& destPath){
    copyAllFiles(sourcePath, destPath);

    QDir sourceDir(sourcePath);
    QStringList files = sourceDir.entryList(QDir::Files);
    foreach(const QString& file, files) {
        QString srcFilePath = sourceDir.filePath(file);
        QFile::remove(srcFilePath);
    }
}

/**
 * @brief Recursively removes a directory and its contents.
 *
 * This function deletes all files and subdirectories within the specified directory.
 * Optionally, it can also remove the directory itself.
 *
 * @param directoryPath The path of the directory to be removed.
 * @param deleteSelf Indicates whether the directory itself should be removed.
 *                   If true, the directory is deleted after its contents are removed.
 *                   If false, only the contents of the directory are deleted.
 * @return Returns true if the operation is successful. Returns false if the directory does not exist,
 *         or if any file or subdirectory cannot be removed.
 */
static bool removeDirectoryRecursively(const QString &directoryPath, bool deleteSelf) {
    QDir dir(directoryPath);

    if (!dir.exists()) {
     return false;
    }

         foreach (QString file, dir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries)) {
         QString fullPath = dir.absoluteFilePath(file);
         if (QFileInfo(fullPath).isDir()) {
             if (!removeDirectoryRecursively(fullPath)) {
                 return false;
             }
         } else {
             if (!QFile::remove(fullPath)) {
                 return false;
             }
         }
     }
    if(deleteSelf)
        return dir.rmdir(".");
    else
        return true;
}