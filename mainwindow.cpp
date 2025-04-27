//
// Created by arseniy on 11.10.2024.
//
#include "mainwindow.h"
#include "ui_MainWindow.h"

#include <QDesktopServices>
#include "QDomDocument"
#include "QFile"
#include "QFileDialog"
#include "QFileInfo"
#include "QMessageBox"
#include "qsaveconfigdialog.h"
#include <QTextStream>

static void copyAllFiles(const QString& sourcePath, const QString& destPath);
static void moveAllFiles(const QString& sourcePath, const QString& destPath);
static bool removeDirectoryRecursively(const QString &directoryPath, bool deleteSelf=true);


MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    setupPlayers();
    setupTracker();

    adjustSize();

    loadSettings();
    saveSettings();

    setupShortcuts();

    connect(ui->actionSave, SIGNAL(triggered(bool)), this, SLOT(saveConfigFile()));
    connect(ui->actionOpen, SIGNAL(triggered(bool)), this, SLOT(loadConfigFile()));
    connect(ui->actionHelp, SIGNAL(triggered(bool)), this, SLOT(openHelp()));
    connect(ui->actionDonate, SIGNAL(triggered(bool)), this, SLOT(openDonate()));
    connect(ui->volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(setVolumeDivider(int)));

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