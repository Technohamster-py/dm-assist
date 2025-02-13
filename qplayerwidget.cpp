//
// Created by arsen on 06.03.2024.
//

#include "qplayerwidget.h"
#include "ui_QPlayer.h"
#include "ui_QPlaylistEdit.h"

#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QFileDialog>
#include <QFileInfo>
#include "QMessageBox"


QPlayer::QPlayer(QWidget *parent, QString title, int numId) :
        QWidget(parent), ui(new Ui::QPlayer) {
    ui->setupUi(this);

    id = numId;
    playlistName = title;

    ui->titleLabel->setText(playlistName);
    ui->numberLabel->setText(QString::number(id));

    playKey = new QShortcut(this);

    m_player = new QMediaPlayer(this);
    playlist = new QMediaPlaylist(this);
    m_player->setPlaylist(playlist);

    playlist->setPlaybackMode(QMediaPlaylist::Loop);

    connect(ui->stopButton, &QPushButton::clicked, m_player, &QMediaPlayer::stop);
    connect(ui->pauseButton, &QPushButton::clicked, m_player, &QMediaPlayer::pause);
    connect(ui->nextButton, &QPushButton::clicked, playlist, &QMediaPlaylist::next);
    connect(ui->prevButton, &QPushButton::clicked, playlist, &QMediaPlaylist::previous);
    connect(playKey, SIGNAL(activated()), this, SLOT(playShortcutTriggered()));
}

QPlayer::QPlayer(QWidget *parent, QFile *xmlFile) :
        QWidget(parent), ui(new Ui::QPlayer) {
    ui->setupUi(this);

    m_player = new QMediaPlayer(this);
    playlist = new QMediaPlaylist(this);
    m_player->setPlaylist(playlist);

    playKey = new QShortcut(this);

    playlist->setPlaybackMode(QMediaPlaylist::Loop);

    connect(ui->playButton, &QPushButton::clicked, m_player, &QMediaPlayer::play);
    connect(ui->stopButton, &QPushButton::clicked, m_player, &QMediaPlayer::stop);
    connect(ui->pauseButton, &QPushButton::clicked, m_player, &QMediaPlayer::pause);
    connect(ui->nextButton, &QPushButton::clicked, playlist, &QMediaPlaylist::next);
    connect(ui->prevButton, &QPushButton::clicked, playlist, &QMediaPlaylist::previous);
    connect(playKey, SIGNAL(activated()), this, SLOT(playShortcutTriggered()));

    loadFromXml(xmlFile);

    ui->titleLabel->setText(playlistName);
    ui->numberLabel->setText(QString::number(id));
}

QPlayer::QPlayer(QWidget *parent, int numId) : QWidget(parent), ui(new Ui::QPlayer) {
    ui->setupUi(this);
    id = numId;
    ui->titleLabel->setText(playlistName);
    ui->numberLabel->setText(QString::number(id));

    QString xmlConfigPath = QCoreApplication::applicationDirPath() + "/configs/" + id + "_playlist.xml";

    m_player = new QMediaPlayer(this);
    playlist = new QMediaPlaylist(this);
    m_player->setPlaylist(playlist);

    playKey = new QShortcut(this);

    playlist->setPlaybackMode(QMediaPlaylist::Loop);

    connect(ui->playButton, &QPushButton::clicked, m_player, &QMediaPlayer::play);
    connect(ui->stopButton, &QPushButton::clicked, m_player, &QMediaPlayer::stop);
    connect(ui->pauseButton, &QPushButton::clicked, m_player, &QMediaPlayer::pause);
    connect(ui->nextButton, &QPushButton::clicked, playlist, &QMediaPlaylist::next);
    connect(ui->prevButton, &QPushButton::clicked, playlist, &QMediaPlaylist::previous);
    connect(playKey, SIGNAL(activated()), this, SLOT(playShortcutTriggered()));

}

QPlayer::~QPlayer() {
    delete ui;
    delete m_player;
    delete playlist;
}

/**
 * Открыть окно изменения плейлиста
 */
void QPlayer::on_editButton_clicked() {
    QPlaylistEdit(nullptr, this).exec();
    ui->titleLabel->setText(playlistName);
}

/**
 * Задать название плейлиста
 * @param name название
 */
void QPlayer::setPlaylistName(QString name) {
    if(name != playlistName){
        playlistName = name;
        emit playlistNameChanged();
    }
}

/**
 * Загрузить существующий плейлист из .xml файла
 * @param xmlFile
 */
void QPlayer::loadFromXml(QFile *xmlFile) {
    if(!xmlFile->open(QIODevice::ReadWrite)){
        QString message = tr("Can not open XML config: ") + xmlFile->errorString();
        QMessageBox::critical(this,
                              tr("File error"),
                              message);
        return;
    }

    xmlConfig.setContent(xmlFile);
    QFileInfo fileInfo(xmlFile->fileName());

    QDomElement playlistNode = xmlConfig.documentElement();
    playlistName = playlistNode.attribute("name");
    id = playlistNode.attribute("id").toInt();
    QDomNodeList tracks = playlistNode.childNodes();

    for (int i = 0; i < tracks.count(); ++i) {
        QDomElement trackNode = tracks.at(i).toElement();
        QString absolutePath = fileInfo.dir().canonicalPath() + "/" + trackNode.firstChild().toText().data();
        playlist->addMedia(QUrl(absolutePath));
    }
    xmlFile->close();
    delete xmlFile;
}

/**
 * Записать плейлист в .xml файл
 * @param pathToXml
 */
void QPlayer::saveToXml(QString pathToXml) {
    QString configFileNamePath;
    if(pathToXml == QCoreApplication::applicationDirPath()){
        configFileNamePath = pathToXml + "/configs/" + QString::number(id) + "_playlist.xml";
    } else
    {
        configFileNamePath = pathToXml;
    }
    QFile config(configFileNamePath);

    if (!config.open(QFile::WriteOnly))
    {
        QMessageBox::critical(this,
                              tr("XML file error"),
                              tr("Can not open XML config file") + config.errorString());
        return;
    }

    QTextStream xmlContent(&config);

    xmlConfig.clear();

    QDomElement root = xmlConfig.createElement("playlist");
    root.setAttribute("name", playlistName);
    root.setAttribute("id", id);
    xmlConfig.appendChild(root);

    QString audioFilePath;
    QDomElement trackNode;

    for (int i = 0; i < playlist->mediaCount(); ++i) {
        audioFilePath = playlist->media(i).canonicalUrl().toString();
        trackNode = xmlConfig.createElement("file");
        trackNode.setNodeValue(audioFilePath);

        root.appendChild(trackNode);
    }

    xmlContent << xmlConfig.toString();
}

void QPlayer::playShortcutTriggered() {
    play();
}

void QPlayer::stop() {
    emit  playerStopped();
    isActive = false;
    m_player->stop();
}

void QPlayer::play() {
    qDebug() << id;
    emit playerStarted();
    isActive = true;
    m_player->play();
}

void QPlayer::on_playButton_clicked() {
    play();
}

/**
 * Задать сочетание клавиш для старта прогирывания
 * @param key сочетание клавиш английской раскладки
 */
void QPlayer::setPlayShortcut(QString key) {
    playKey->setKey(key);
}

void QPlayer::edit() {
    on_editButton_clicked();
}



////////////////////////////////////////////////////
//               QPlaylistEdit                    //
////////////////////////////////////////////////////

QPlaylistEdit::QPlaylistEdit(QWidget *parent, QPlayer *player) :
        QDialog(parent), ui(new Ui::QPlaylistEdit) {
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->titleEdit->setText(player->getPlaylistName());

    m_player = player;
    m_playlistModel = new QStandardItemModel(this);
    m_playlistModel->setHorizontalHeaderLabels(QStringList() << tr("Track title") << tr("File path"));

    ui->playlistView->setModel(m_playlistModel);
    ui->playlistView->hideColumn(1);
    ui->playlistView->verticalHeader()->setVisible(false);
    ui->playlistView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->playlistView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->playlistView->horizontalHeader()->setStretchLastSection(true);

    displayPlaylist();
}

QPlaylistEdit::~QPlaylistEdit() {
    delete ui;
}

void QPlaylistEdit::on_addButton_clicked() {
    QStringList files = QFileDialog::getOpenFileNames(this,
                                                      tr("Add tracks"),
                                                      QString(),
                                                      tr("Audio files (*.mp3)"));

    foreach (QString filePath, files){
        QList<QStandardItem *> items;

        QString localDirPath = QStandardPaths::writableLocation(QStandardPaths::MusicLocation) + "/dm_assis_files/playlists/" + QString::number(m_player->getId()) + "_playlist";
        qDebug() << localDirPath;

        QStringList fileDirs = filePath.split("/");
        QString fileName = fileDirs.at(fileDirs.size()-1);
        QString copiedFilePath = localDirPath + "/" + fileName;

        if(!QDir(localDirPath).exists()){
            QDir().mkpath(localDirPath);
        }

        //qDebug() << copiedFilePath;
        if(!QDir(copiedFilePath).exists())
            QFile::copy(filePath, copiedFilePath);

        items.append((new QStandardItem(QDir(copiedFilePath).dirName())));
        items.append(new QStandardItem(copiedFilePath));
        m_playlistModel->appendRow(items);
        m_player->playlist->addMedia(QUrl(copiedFilePath));
    }
}

void QPlaylistEdit::on_removeButton_clicked() {
    int index = ui->playlistView->selectionModel()->selection().indexes()[0].row();
    qDebug() << "Index " << index;
    m_playlistModel->removeRow(index);
    m_player->playlist->removeMedia(index);
}

void QPlaylistEdit::on_buttonBox_accepted() {
    m_player->setPlaylistName(ui->titleEdit->text());
    accept();
}

void QPlaylistEdit::on_buttonBox_rejected() {
    reject();
}

void QPlaylistEdit::displayPlaylist() {
    for (int i = 0; i < m_player->playlist->mediaCount(); ++i) {
        QList<QStandardItem *> items;
        QString filePath =  m_player->playlist->media(i).canonicalUrl().toString();
        items.append((new QStandardItem(QDir(filePath).dirName())));
        items.append(new QStandardItem(filePath));
        m_playlistModel->appendRow(items);
    }
}

void QPlaylistEdit::on_folderButton_clicked() {
    QString folderName = QFileDialog::getExistingDirectory(this,
                                                           tr("Open folder"));

}

void QPlaylistEdit::on_uploadButton_clicked() {
    QString configFileName = QFileDialog::getOpenFileName();

    if (!configFileName.isEmpty())
    {
        m_player->loadFromXml(new QFile(configFileName));
    }
}
