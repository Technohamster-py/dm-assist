//
// Created by arseniy on 11.10.2024.
//
#include "mainwindow.h"
#include "ui_MainWindow.h"
#include "map-widget/mapview.h"

#include <QDesktopServices>
#include "QDomDocument"
#include "QFile"
#include "QFileDialog"
#include <QColorDialog>
#include <QCheckBox>
#include "QFileInfo"
#include "QMessageBox"
#include "qsaveconfigdialog.h"
#include <QTextStream>
#include <QSpinBox>

#include <QDebug>

static void copyAllFiles(const QString& sourcePath, const QString& destPath);
static void moveAllFiles(const QString& sourcePath, const QString& destPath);
static bool removeDirectoryRecursively(const QString &directoryPath, bool deleteSelf=true);


MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    setupToolbar();
    setupPlayers();
    setupTracker();
    setupMaps();

    adjustSize();

    loadSettings();
    saveSettings();

    setupShortcuts();

    connect(ui->actionSave, SIGNAL(triggered(bool)), this, SLOT(saveConfigFile()));
    connect(ui->actionOpen, SIGNAL(triggered(bool)), this, SLOT(loadConfigFile()));
    connect(ui->actionHelp, SIGNAL(triggered(bool)), this, SLOT(openHelp()));
    connect(ui->actionDonate, SIGNAL(triggered(bool)), this, SLOT(openDonate()));
    connect(ui->volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(setVolumeDivider(int)));

    showMaximized();
}

/**
 * Закрытие главного окна
 * @details При закрытии окна производится сохранение конфигурационного файла
 */
MainWindow::~MainWindow() {
//    QMessageBox::StandardButton reply = QMessageBox::question(this,
//                                                              tr("Сохранение конфигурации"),
//                                                              tr("Сохранить проект?"),
//                                                              QMessageBox::Yes | QMessageBox::No);
//
//    if(reply == QMessageBox::Yes)
//        saveConfigFile();
    saveSettings();
    foreach(QPlayer* player, players){
        removeDirectoryRecursively(player->getLocalDirPath());
        delete player;
    }
    delete ui;
}


/**
 * Конфигурирование плееров
 *
 * @details подключение всех плееров к их виджетам.
 * Подключение сигналов о начале проигрывания, изменения и остановке к плеерам.
 */
void MainWindow::setupPlayers() {
     for (int i = 0; i < 9; ++i) {
         auto *player = new QPlayer(this, i, QString("Player %1").arg(i + 1));
         players.append(player);
     }

    for (int i = 0; i < players.size(); ++i) {
        ui->musicLayout->insertWidget(i, players[i]);
        connect(players[i], SIGNAL(playerStarted(int)), this, SLOT(stopOtherPlayers(int)));
    }
}

void MainWindow::setupShortcuts() {
    for (int i = 0; i < players.size(); ++i) {
        QString key = QString("Ctrl+%1").arg(i);
        players[i]->setPlayShortcut(key);
    }
}

/**
 * Остановка всех плееров
 */
void MainWindow::stopAll() {
    for (int i = 0; i < 9; ++i) {
        players[i]->stop();
    }
}

void MainWindow::loadConfigFile() {
    QString fileName = QFileDialog::getOpenFileName(this,
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
            QString absolutePath = playerNode.firstChild().toText().data();     // Получаем абсолютный путь к папке с плейлистом

            int playerId = playerNode.attribute("id").toInt();                  ///< Получаем Id плейлиста

            /// Получаем имя папки плейлиста (имя самого плейлиста)
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
                for (const QString &fileName : fileNames) {
                    fullPaths.append(playerDir.absoluteFilePath(fileName));
                }
                players[playerId]->addMedia(fullPaths);
            }
        }
        configFile.close();
    }
}

/**
 * Сохранение конфигурационного файла
 * @details У пользователя запрашивается название проекта и корневая папка сохранения.
 * После этого все файлы из рабочей папки копируются в папку сохранения и создается .xml конфиг
 */
void MainWindow::saveConfigFile() {
    SaveConfigDialog dialog(this);
    QString fileName = "";

    if (dialog.exec() == QDialog::Accepted)
        fileName = dialog.filename;

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

        QString playlistPath = baseDir + players[i]->getPlaylistName();
        moveAllFiles(players[i]->getLocalDirPath(), playlistPath);

        QDomText textNode = configDocument.createTextNode(playlistPath);
        playlist_node.appendChild(textNode);

        root.appendChild(playlist_node);
    }

    xmlContent << configDocument.toString();
    configFile.close();
}

void MainWindow::saveSettings() {
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
    settings.setValue(paths.general.dir, workingDir);
    settings.setValue(paths.general.volume, ui->volumeSlider->value());
    settings.sync();
}

void MainWindow::loadSettings() {
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
    /// General
    workingDir = settings.value(paths.general.dir, workingDir).toString();
    QDir dir(workingDir);
    if (!dir.exists())
        dir.mkpath(".");
    /// Music
    for (QPlayer *player : players) {
        player->setAudioOutput(settings.value(paths.general.audioDevice, 0).toInt());
    }
    ui->volumeSlider->setValue(settings.value(paths.general.volume, 100).toInt());
    ///Language
    changeLanguage(settings.value(paths.general.lang, "ru_RU").toString());
    /// Initiative tracker
    initiativeTrackerWidget->setHpDisplayMode(settings.value(paths.initiative.hpBarMode, 0).toInt());
    int initiativeFields = settings.value(paths.initiative.fields, 7).toInt();
    initiativeTrackerWidget->setSharedFieldVisible(0, initiativeFields & iniFields::name);
    initiativeTrackerWidget->setSharedFieldVisible(1, initiativeFields & iniFields::init);
    initiativeTrackerWidget->setSharedFieldVisible(2, initiativeFields & iniFields::ac);
    initiativeTrackerWidget->setSharedFieldVisible(3, initiativeFields & iniFields::hp);
    initiativeTrackerWidget->setSharedFieldVisible(4, initiativeFields & iniFields::maxHp);
    initiativeTrackerWidget->setSharedFieldVisible(5, initiativeFields & iniFields::del);
}

void MainWindow::on_actionSettings_triggered() {
    saveSettings();
    if(!settingsDialog)
    {
        settingsDialog = new SettingsDialog(ORGANIZATION_NAME, APPLICATION_NAME, this);
    }
    settingsDialog->exec();
    loadSettings();
}

void MainWindow::stopOtherPlayers(int exeptId) {
    for (int i = 0; i < 9; ++i) {
        if (players[i]->getPlaylistId() != exeptId)
            players[i]->stop();
    }
}

void MainWindow::changeLanguage(const QString &languageCode) {
    qApp->removeTranslator(&translator);
    if (translator.load(QCoreApplication::applicationDirPath() + "/translations/dm-assist_" + languageCode + ".qm"))
    {
        qApp->installTranslator(&translator);
        currentLanguage = languageCode;
        ui->retranslateUi(this);
    }
}

void MainWindow::openHelp() {
    QUrl url("https://github.com/Technohamster-py/dm-assist/wiki/%D0%9D%D0%B0%D1%87%D0%B0%D0%BB%D0%BE");
    QDesktopServices::openUrl(url);
}

void MainWindow::openDonate() {
    QUrl url("https://pay.cloudtips.ru/p/8f6d339a");
    QDesktopServices::openUrl(url);
}

void MainWindow::setVolumeDivider(int value) {
    for (int i = 0; i < 9; ++i) {
        players[i]->setVolumeDivider(value);
    }
}

void MainWindow::setupTracker() {
    initiativeTrackerWidget = new QInitiativeTrackerWidget(this);
    ui->trackerLayout->addWidget(initiativeTrackerWidget);
}

void MainWindow::createNewMapTab() {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Map Image"),
                                                    QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                                                    tr("Images (*.png *.jpg *.bmp)"));

    if (!fileName.isEmpty()){
        QFileInfo fileInfo(fileName);
        MapView *view = new MapView(this);
        view->loadMapImage(fileName);
        mapTabWidget->addTab(view, fileInfo.fileName());
        updateVisibility();
        mapTabWidget->setCurrentIndex(mapTabWidget->count()-1);
    }
    mapTabWidget->setCurrentIndex(mapTabWidget->count() - 1);
}

void MainWindow::setupMaps() {
    mapTabWidget = new TabWidget(this);
    ui->mainViewLayout->addWidget(mapTabWidget);
    ui->phLayout->setAlignment(Qt::AlignCenter);
    connect(ui->openMapButton, &QPushButton::clicked, this, &MainWindow::createNewMapTab);
    connect(mapTabWidget, &TabWidget::newTabRequested, this, &MainWindow::createNewMapTab);
    connect(ui->actionAddMap, &QAction::triggered, this, &MainWindow::createNewMapTab);
    connect(mapTabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::deleteMapTab);
    connect(mapTabWidget, &TabWidget::share, this, &MainWindow::openSharedMapWindow);

    updateVisibility();
}

void MainWindow::updateVisibility() {
    bool hasTabs = mapTabWidget->count() > 0;
    mapTabWidget->setVisible(hasTabs);
    ui->placeHolderWidget->setVisible(!hasTabs);
}

void MainWindow::deleteMapTab(int index) {
    QWidget *widget = mapTabWidget->widget(index);
    mapTabWidget->removeTab(index);
    delete widget;
    updateVisibility();
}

void MainWindow::setCalibrationMode() {
    MapView* currentView = qobject_cast<MapView*>(mapTabWidget->currentWidget());
    if (currentView){
        currentView->setActiveTool(calibrationTool);
    }
}

void MainWindow::setMeasureMode(bool checked) {
    MapView* currentView = qobject_cast<MapView*>(mapTabWidget->currentWidget());
    if (currentView){
        if (checked)
            currentView->setActiveTool(rulerMapTool);
        else
            currentView->setActiveTool(nullptr);
    }
}

void MainWindow::setupToolbar() {
    ui->toolBar->setMovable(false);

    calibrationTool = new CalibrationTool(this);
    fogTool = new FogTool(this);
    lightTool = new LightTool(this);
    rulerMapTool = new RulerMapTool(this);
    lineShapeTool = new LineShapeTool();
    circleShapeTool = new CircleShapeTool();
    squareShapeTool = new SquareShapeTool();
    triangleShapeTool = new TriangleShapeTool();

    QActionGroup *toolGroup = new QActionGroup(this);
    toolGroup->setExclusive(true);

    /// Ruler too
    QToolButton *rulerButton = new QToolButton(this);
    rulerButton->setCheckable(true);
    rulerButton->setToolTip(tr("ruler"));

    QAction *rulerAction = new QAction(this);
    rulerAction->setCheckable(true);
    rulerAction->setChecked(false);
    toolGroup->addAction(rulerAction);
    rulerAction->setIcon(QIcon(":/map/ruler.svg"));
    rulerButton->setDefaultAction(rulerAction);

    rulerButton->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(rulerButton, &QToolButton::customContextMenuRequested, this, [=](const QPoint &pos) {
        QMenu contextMenu;
        QAction *calibrateAct = contextMenu.addAction(tr("calibrate"));
        QAction *chosen = contextMenu.exec(rulerButton->mapToGlobal(pos));
        if (chosen == calibrateAct) {
            // временно активируем CalibrationTool
            setCalibrationMode();
            // отключим кнопку линейки на время
            rulerAction->setChecked(false);
        }
    });
    connect(rulerAction, SIGNAL(triggered(bool)), this, SLOT(setMeasureMode(bool)));
    connect(calibrationTool, &AbstractMapTool::finished, [=]() {
        setMeasureMode(true);
        rulerAction->setChecked(true);
    });
    ui->toolBar->addWidget(rulerButton);

    /// Fog-hide tool
    QAction* fogHideAction = new QAction(this);
    fogHideAction->setCheckable(true);
    fogHideAction->setIcon(QIcon(":/map/fog_hide.svg"));
    toolGroup->addAction(fogHideAction);

    QToolButton* fogHideButton = new QToolButton(this);
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
        setFogTool(checked, FogTool::Hide);
    });

    /// Fog-reveal tool
    QAction* fogRevealAction = new QAction(this);
    fogRevealAction->setCheckable(true);
    fogRevealAction->setIcon(QIcon(":/map/fog_reveal.svg"));
    toolGroup->addAction(fogRevealAction);

    QToolButton* fogRevealButton = new QToolButton(this);
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
        setFogTool(checked, FogTool::Reveal);
    });

    ui->toolBar->addSeparator();

    /// Light tool
    QAction* lightAction = new QAction(this);
    lightAction->setCheckable(true);
    lightAction->setIcon(QIcon(":/map/torch.svg"));
    toolGroup->addAction(lightAction);

    QToolButton* lightButton = new QToolButton(this);
    lightButton->setCheckable(true);
    lightButton->setToolTip(tr("Edit light sources"));
    lightButton->setDefaultAction(lightAction);
    ui->toolBar->addWidget(lightButton);

    QSpinBox *brightRadiusBox = new QSpinBox;
    brightRadiusBox->setToolTip(tr("Bright radius"));
    brightRadiusBox->setRange(1, 500);
    brightRadiusBox->setValue(20);
    lightTool->setBrightRadius(brightRadiusBox->value());
    ui->toolBar->addWidget(brightRadiusBox);

    QSpinBox *dimRadiusBox = new QSpinBox;
    dimRadiusBox->setToolTip(tr("Dim radius"));
    dimRadiusBox->setRange(1, 1000);
    dimRadiusBox->setValue(40);
    lightTool->setDimRadius(dimRadiusBox->value());
    ui->toolBar->addWidget(dimRadiusBox);

    QPushButton *lightColorBtn = new QPushButton();
    lightColorBtn->setIcon(QIcon(":/map/palette.svg"));
    ui->toolBar->addWidget(lightColorBtn);

    connect(lightAction, &QAction::triggered, this, &MainWindow::setLightTool);

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

    QCheckBox *fogUpdateBox = new QCheckBox(tr("Update fog"));
    fogUpdateBox->setChecked(false);
    ui->toolBar->addWidget(fogUpdateBox);

    connect(fogUpdateBox, &QCheckBox::toggled, lightTool, &LightTool::setAutoUpdateFog);

    ui->toolBar->addSeparator();

    /// Spells
    /// LineShapeTool
    QAction* lineAction = new QAction(this);
    lineAction->setCheckable(true);
    lineAction->setIcon(QIcon(":/map/line.svg"));
    toolGroup->addAction(lineAction);

    QToolButton* lineButton = new QToolButton(this);
    lineButton->setCheckable(true);
    lineButton->setToolTip(tr("Draw line"));
    lineButton->setDefaultAction(lineAction);
    ui->toolBar->addWidget(lineButton);

    connect(lineAction, &QAction::triggered, this, [=](bool checked){
        MapView* currentView = qobject_cast<MapView*>(mapTabWidget->currentWidget());
        if (checked)
            currentView->setActiveTool(lineShapeTool);
        else
            currentView->setActiveTool(nullptr);
    });

    /// CircleShapeTool
    QAction* circleAction = new QAction(this);
    circleAction->setCheckable(true);
    circleAction->setIcon(QIcon(":/map/sphere.svg"));
    toolGroup->addAction(circleAction);

    QToolButton* circleButton = new QToolButton(this);
    circleButton->setCheckable(true);
    circleButton->setToolTip(tr("Draw circle"));
    circleButton->setDefaultAction(circleAction);
    ui->toolBar->addWidget(circleButton);

    connect(circleAction, &QAction::triggered, this, [=](bool checked){
        MapView* currentView = qobject_cast<MapView*>(mapTabWidget->currentWidget());
        if (checked)
            currentView->setActiveTool(circleShapeTool);
        else
            currentView->setActiveTool(nullptr);
    });

    /// SquareShapeTool
    QAction* squareAction = new QAction(this);
    squareAction->setCheckable(true);
    squareAction->setIcon(QIcon(":/map/cube.svg"));
    toolGroup->addAction(squareAction);

    QToolButton* squareButton = new QToolButton(this);
    squareButton->setCheckable(true);
    squareButton->setToolTip(tr("Draw square"));
    squareButton->setDefaultAction(squareAction);
    ui->toolBar->addWidget(squareButton);

    connect(squareAction, &QAction::triggered, this, [=](bool checked){
        MapView* currentView = qobject_cast<MapView*>(mapTabWidget->currentWidget());
        if (checked)
            currentView->setActiveTool(squareShapeTool);
        else
            currentView->setActiveTool(nullptr);
    });

    /// TriangleShapeTool
    QAction* triangleAction = new QAction(this);
    triangleAction->setCheckable(true);
    triangleAction->setIcon(QIcon(":/map/cone.svg"));
    toolGroup->addAction(triangleAction);

    QToolButton* triangleButton = new QToolButton(this);
    triangleButton->setCheckable(true);
    triangleButton->setToolTip(tr("Draw triangle"));
    triangleButton->setDefaultAction(triangleAction);
    ui->toolBar->addWidget(triangleButton);

    connect(triangleAction, &QAction::triggered, this, [=](bool checked){
        MapView* currentView = qobject_cast<MapView*>(mapTabWidget->currentWidget());
        if (checked)
            currentView->setActiveTool(triangleShapeTool);
        else
            currentView->setActiveTool(nullptr);
    });

    /// Shape color button
    QPushButton *ShapeToolColorButton = new QPushButton();
    ShapeToolColorButton->setIcon(QIcon(":/map/palette.svg"));
    ui->toolBar->addWidget(ShapeToolColorButton);

    connect(ShapeToolColorButton, &QPushButton::clicked, this, [=]() {
        QColor chosen = QColorDialog::getColor(lightTool->color(), this);
        if (chosen.isValid()) {
            lineShapeTool->setColor(chosen);
            circleShapeTool->setColor(chosen);
            squareShapeTool->setColor(chosen);
            triangleShapeTool->setColor(chosen);
        }
    });
}

void MainWindow::openSharedMapWindow(int index) {
    MapView* currentView = qobject_cast<MapView*>(mapTabWidget->widget(index));
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

void MainWindow::setFogTool(bool checked, FogTool::Mode mode) {
    MapView* currentView = qobject_cast<MapView*>(mapTabWidget->currentWidget());
    if (currentView){
        if (!checked){
            currentView->setActiveTool(nullptr);
        } else{
            fogTool->setMode(mode);
            currentView->setActiveTool(fogTool);
        }
    }
}

void MainWindow::coverMapWithFog(bool hide) {
    MapView* currentView = qobject_cast<MapView*>(mapTabWidget->currentWidget());
    MapScene* scene = currentView->getScene();

    if (hide)
        fogTool->hideAll(scene);
    else
        fogTool->revealAll(scene);
}

void MainWindow::setLightTool(bool checked) {
    MapView* currentView = qobject_cast<MapView*>(mapTabWidget->currentWidget());
    if (checked)
        currentView->setActiveTool(lightTool);
    else
        currentView->setActiveTool(nullptr);
}


/**
 * Копирование всех фалов из папки sourcePath в папку destPath
 * @param sourcePath изначальное расположение
 * @param destPath целевое расположение
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
 * Перемещение всех файлов из папки sourcePath в папку destPath
 * @param sourcePath начальное расположение
 * @param destPath целевое расположение
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


static bool removeDirectoryRecursively(const QString &directoryPath, bool deleteSelf) {
    QDir dir(directoryPath);

    // Проверяем, существует ли директория
    if (!dir.exists()) {
     return false; // Если директория не существует, возвращаем false
    }

    // Получаем список всех файлов и поддиректорий
         foreach (QString file, dir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries)) {
         QString fullPath = dir.absoluteFilePath(file);
         if (QFileInfo(fullPath).isDir()) {
             // Если это поддиректория, рекурсивно вызываем функцию
             if (!removeDirectoryRecursively(fullPath)) {
                 return false; // Если не удалось удалить поддиректорию, возвращаем false
             }
         } else {
             // Удаляем файл
             if (!QFile::remove(fullPath)) {
                 return false; // Если файл не удалось удалить, возвращаем false
             }
         }
     }

    // После удаления всех файлов и поддиректорий удаляем саму директорию
    if(deleteSelf)
        return dir.rmdir("."); // dir.rmdir(".") удаляет саму папку
    else
        return true;
}