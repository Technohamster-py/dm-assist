 //
// Created by arseniy on 11.10.2024.
//
#include "mainwindow.h"
#include "ui_MainWindow.h"
#include "QFileDialog"
#include "QMessageBox"
#include "QFile"
#include "QFileInfo"
#include "QDomDocument"
#include "qsaveconfigdialog.h"


static void copyAllFiles(const QString& sourcePath, const QString& destPath);
static void moveAllFiles(const QString& sourcePath, const QString& destPath);
static bool removeDirectoryRecursively(const QString &directoryPath, bool deleteSelf=true);


MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    playerWidgetsList = {ui->player1, ui->player2, ui->player3, ui->player4, ui->player5, ui->player6, ui->player7, ui->player8, ui->player9};
    playButtonsList = {ui->play1, ui->play2, ui->play3, ui->play4, ui->play5, ui->play6, ui->play7, ui->play8, ui->play9};
    editButtonList = {ui->edit1, ui->edit2, ui->edit3, ui->edit4, ui->edit5, ui->edit6, ui->edit7, ui->edit8, ui->edit9};
    stopShortcut = new QShortcut(this);
    configurePlayers();
}

/**
 * Закрытие главного окна
 * @details При закрытии окна производится сохранение конфигурационного файла
 */
MainWindow::~MainWindow() {
    QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                              "Сохранение конфигурации",
                                                              "Сохранить проект?",
                                                              QMessageBox::Yes | QMessageBox::No);

    if(reply == QMessageBox::Yes)
        saveConfigFile();

    foreach(QPlayer* player, playerList){
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
void MainWindow::configurePlayers() {

    for (int i = 0; i < 9; ++i) {
        playerList.append(new QPlayer(playerWidgetsList[i], i+1));
    }

    for (int i = 0; i < 9; ++i) {
        /// При запуске одного плеера останавливаются все остальные
        connect(playerList[i], SIGNAL(playerStarted()), this, SLOT(stopAll()));
        playerList[i]->setPlayShortcut(QString("Ctrl+%1").arg(QString::number(i+1)));
        connect(playButtonsList[i], SIGNAL(clicked(bool)), playerList[i], SLOT(play()));
        connect(editButtonList[i], SIGNAL(clicked(bool)), playerList[i], SLOT(edit()));
    }
    stopShortcut->setKey(QString("Ctrl+0"));
    connect(stopShortcut, SIGNAL(activated()), this, SLOT(stopAll()));
}

/**
 * Остановка всех плееров
 */
void MainWindow::stopAll() {
    for (int i = 0; i < 9; ++i) {
        playerList[i]->stop();
    }
}

void MainWindow::loadConfigFile() {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open player config file"),
                                                    QStandardPaths::writableLocation(QStandardPaths::MusicLocation) + "/dm_assis_files/saves",
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
        QDomNodeList players = mainNode.childNodes();

        for (int i=0; i<players.count(); i++){
            QDomElement playerNode = players.at(i).toElement();
            QString absolutePath = playerNode.firstChild().toText().data();     // Получаем абсолютный путь к папке с плейлистом

            int playerId = playerNode.attribute("id").toInt();                  ///< Получаем Id плейлиста

            /// Получаем имя папки плейлиста (имя самого плейлиста)
            QFileInfo dirInfo(absolutePath);
            if (dirInfo.isDir()){
                playerList[playerId]->setPlaylistName(dirInfo.fileName());
            }

            QDir playerDir(absolutePath);
            if (!playerDir.exists()) {
                playerList[playerId]->addMedia(QStringList());
            }
            else{
                QStringList fileNames = playerDir.entryList(QDir::Files);
                QStringList fullPaths;
                for (const QString &fileName : fileNames) {
                    fullPaths.append(playerDir.absoluteFilePath(fileName));
                }
                playerList[playerId]->addMedia(fullPaths);
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

        QString playlistPath = baseDir + playerList[i]->getPlaylistName();
        moveAllFiles(playerList[i]->getLocalDirPath(), playlistPath);

        QDomText textNode = configDocument.createTextNode(playlistPath);
        playlist_node.appendChild(textNode);

        root.appendChild(playlist_node);
    }

    xmlContent << configDocument.toString();
    configFile.close();
}

void MainWindow::on_actionOpen_triggered() {
    loadConfigFile();
}

void MainWindow::on_actionSave_triggered() {
    saveConfigFile();
}

void MainWindow::saveSettings() {
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
    settings.sync();
}

void MainWindow::loadSettings() {
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
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
//     qDebug() << "Директория не существует:" << directoryPath;
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
//                 qDebug() << "Не удалось удалить файл:" << fullPath;
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