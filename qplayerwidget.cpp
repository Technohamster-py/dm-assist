//
// Created by arsen on 06.03.2024.
//

#include "qplayerwidget.h"
#include "ui_QPlayer.h"
#include "ui_QPlaylistEdit.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QPushButton>

#include <QRegExp>
#include <QRegExpValidator>

#include "bass/bass.h"

#define BASS_DEVICE_INDEX 1 // Можно сделать параметром, если нужно


////////////////////////////////////////////////
/////////       QPlayer                  ///////
////////////////////////////////////////////////

QPlayer::QPlayer(QWidget *parent, int id, QString title)
        : QWidget(parent),
          ui(new Ui::QPlayer),
          id(id),
          playlistName(std::move(title))
{
    ui->setupUi(this);
    ui->titleLabel->setText(playlistName);
    ui->numberLabel->setText(QString::number(id));

    setAcceptDrops(true);

    localDir = QStandardPaths::writableLocation(QStandardPaths::MusicLocation) + QString("/dm_assist_files/playlists/tmp/%1").arg(playlistName);
    QDir().mkpath(localDir);

    BASS_Free();

    if (!BASS_Init(BASS_DEVICE_INDEX, 44100, 0, nullptr, nullptr)) {
        auto err = BASS_ErrorGetCode();
        QMessageBox::critical(this, "BASS Init Failed",
                              QString("Could not initialize BASS on selected device.\nError code: %1").arg(err));
        return;
    }

    playKey = new QShortcut(this);

    connect(ui->playButton, &QPushButton::clicked, this, &QPlayer::on_playButton_clicked);
    connect(playKey, SIGNAL(activated()), this, SLOT(playShortcutTriggered()));
    connect(ui->volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(changeVolume(int)));
}

QPlayer::~QPlayer() {
    stop();
    freeStreams();
    BASS_Free();
    delete ui;
}

void QPlayer::setPlaylistName(const QString &name) {
    if(name != playlistName){
        playlistName = name;
        ui->titleLabel->setText(name);
        QFileInfo fileInfo(localDir);
        QString parentPath = fileInfo.dir().absolutePath();
        QString newPath = QDir(parentPath).filePath(playlistName);
        QFile::rename(localDir, newPath);

        QDir newDir(newPath);
        QStringList newFileNames = newDir.entryList(QDir::Files);
        filePaths.clear();
        addMedia(newFileNames);
        localDir = newPath;
        emit playlistNameChanged();
    }
}

void QPlayer::setVolumeDivider(int value) {
    volumeDivider = value;
    if (volumeDivider > 100)
        volumeDivider = 100;
    if (volumeDivider < 0)
        volumeDivider = 0;
    changeVolume(ui->volumeSlider->value());
}

void QPlayer::setPlayShortcut(QString key) {
    playKey->setKey(key);
}

void QPlayer::playShortcutTriggered() {
    play();
}

void QPlayer::addMedia(const QStringList &files) {
    for (const QString &file : files) {
        QFileInfo info(file);
        QString dest = localDir + "/" + info.fileName();

        QDir destDir(localDir);
        if (!destDir.exists())
            destDir.mkpath(".");

        QFile::copy(file, dest);
        filePaths.append(dest);
    }

    emit localDirPathChanged();
}

void QPlayer::on_playButton_clicked() {
    if (isActive)
        stop();
    else
        play();
}

void QPlayer::on_editButton_clicked() {
    edit();
}

void QPlayer::edit() {
    QPlaylistEdit editor(this, filePaths, playlistName);
    if (editor.exec() == QDialog::Accepted) {
        QStringList newList = editor.getUpdatedPlaylist();
        QDir dir(localDir);
        for (const QFileInfo &file : dir.entryInfoList(QDir::Files)) {
            QFile::remove(file.absoluteFilePath());
        }

        filePaths.clear();

        setPlaylistName(editor.getPlaylistName());

        addMedia(newList); // это скопирует в локальную папку и обновит filePaths
    }
}

void QPlayer::play() {
    emit playerStarted(id);
    freeStreams();  // очистить
    isActive = true;

    if (filePaths.isEmpty()) return;

    for (const QString &file: filePaths) {
        HSTREAM stream = BASS_StreamCreateFile(FALSE, file.toStdString().c_str(), 0, 0, 0);
        streams.append(stream);
    }

    currentTrackIndex = 0;
    playTrackAt(currentTrackIndex);
}

void QPlayer::playTrackAt(int index) {
    if (index < 0 || index >= streams.size()) return;

    stop(); // на всякий случай

    stream = streams[index];
    BASS_ChannelPlay(stream, FALSE);

    // Установка синхронизации на окончание трека
    BASS_ChannelSetSync(stream, BASS_SYNC_END, 0, [](HSYNC, DWORD handle, DWORD, void *user) {
        QPlayer *self = static_cast<QPlayer*>(user);
        QMetaObject::invokeMethod(self, "playNextTrack", Qt::QueuedConnection);
    }, this);

    isActive = true;
    ui->playButton->setStyleSheet("background-color: #a5bfda;");
    QIcon stopIcon(":/player/stop.svg");
    ui->playButton->setIcon(stopIcon);
}

void QPlayer::playNextTrack() {
    ++currentTrackIndex;

    if (currentTrackIndex >= streams.size()) {
        currentTrackIndex = 0; // или stop(), если не нужно зацикливать
    }

    playTrackAt(currentTrackIndex);
}

void QPlayer::stop() {
    for (HSTREAM stream : streams) {
        BASS_ChannelStop(stream);
    }
    isActive = false;
    emit playerStopped();
    ui->playButton->setStyleSheet("background: transparent;");
    QIcon playIcon(":/player/play.svg");
    ui->playButton->setIcon(playIcon);
}

void QPlayer::freeStreams() {
    for (HSTREAM stream : streams) {
        BASS_StreamFree(stream);
    }
    streams.clear();
}

void QPlayer::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void QPlayer::dropEvent(QDropEvent *event) {
    QStringList files;
    for (const QUrl &url : event->mimeData()->urls()) {
        QString path = url.toLocalFile();
        QFileInfo info(path);
        if (info.exists() && info.isFile() && (info.suffix().toLower() == "mp3" || info.suffix().toLower() == "wav")) {
            files.append(path);
        }
    }

    if (!files.isEmpty()) {
        addMedia(files);
    }
}

/**
 * Установить имя локальной папки
 * @param localDirPath путь к локальной папке
 */
void QPlayer::setLocalDirPath(QString localDirPath) {
    localDir = localDirPath + playlistName;
    emit localDirPathChanged();
}

QStringList QPlayer::availableAudioDevices() const {
    QStringList list;
    BASS_DEVICEINFO info;
    for (int i = 1; BASS_GetDeviceInfo(i, &info); i++) {
        if (info.flags & BASS_DEVICE_ENABLED)
            list << QString::fromLocal8Bit(info.name);
    }
    return list;
}

QString QPlayer::currentDeviceName() const {
    if (m_deviceIndex < 0) return {};
    BASS_DEVICEINFO info;
    if (BASS_GetDeviceInfo(m_deviceIndex, &info))
        return QString::fromLocal8Bit(info.name);
    return {};
}

/**
 * Усановить громкость плеера
 * @param volume громкость в процентах (0% - 100%)
 */
void QPlayer::changeVolume(int volume) {

    float trueVolume = volume / 100;

    if (trueVolume > 1.0f)
        trueVolume = 1.0f;
    if (trueVolume < 0.0f)
        trueVolume = 0.0f;

    BASS_ChannelSetAttribute(stream, BASS_ATTRIB_VOL, (trueVolume * static_cast<float>(volumeDivider) / 100.0f));
}

void QPlayer::setAudioOutput(const QString &deviceName) {
    // Найдём индекс по имени
    BASS_DEVICEINFO info;
    int found = -1;

    for (int i = 1; BASS_GetDeviceInfo(i, &info); i++) {
        if (QString::fromLocal8Bit(info.name) == deviceName && (info.flags & BASS_DEVICE_ENABLED)) {
            found = i;
            break;
        }
    }

    if (found == -1) {
        return;
    }

    // Если уже такой активен — ничего не делаем
    if (m_deviceIndex == found) return;

    stop();
    freeStreams();
    BASS_Free();

    if (!BASS_Init(found, 44100, 0, nullptr, nullptr)) {
        QMessageBox::critical(this, "BASS Init Failed", "Could not initialize BASS on selected device.");
        return;
    }

    m_deviceIndex = found;
}

void QPlayer::setAudioOutput(int deviceIndex) {
    if (deviceIndex == -1) {
        return;
    }

    // Если уже такой активен — ничего не делаем
    if (m_deviceIndex == deviceIndex) return;

    stop();
    freeStreams();
    BASS_Free();

    if (!BASS_Init(deviceIndex, 44100, 0, nullptr, nullptr)) {
        QMessageBox::critical(this, "BASS Init Failed", "Could not initialize BASS on selected device.");
        return;
    }

    m_deviceIndex = deviceIndex;
}

/**
 * Установить громкость извне.
 * @param volume громкость в процентах (0-100)
 *
 * @details Метод устанавливает слайдер громкости на нужную позицию. После этого уже вызывается слот changeVolume.
 * Это сделано для избежания бесконечного цикла сигнал-слот
 */
void QPlayer::setVolume(int volume) {
    if (volume > 100)
        volume = 100;
    if (volume < 0)
        volume = 0;
    ui->volumeSlider->setValue(volume);
}

////////////////////////////////////////////////
/////////       QPlaylistEdit            ///////
////////////////////////////////////////////////

QPlaylistEdit::QPlaylistEdit(QWidget *parent, const QStringList &tracks, QString title)
        : QDialog(parent), ui(new Ui::QPlaylistEdit)
{
    ui->setupUi(this);
    resize(400, 300);

    QRegExp regex("[A-Za-z0-9\\-_ ]+");
    QRegExpValidator *validator = new QRegExpValidator(regex, ui->titleEdit);
    ui->titleEdit->setValidator(validator);

    ui->titleEdit->setText(title);
    ui->playlistWidget->addItems(tracks);
    ui->playlistWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->playlistWidget->setDragDropMode(QAbstractItemView::InternalMove);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QPlaylistEdit::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QPlaylistEdit::reject);
}

QPlaylistEdit::~QPlaylistEdit() = default;

void QPlaylistEdit::on_addButton_clicked() {
    QStringList files = QFileDialog::getOpenFileNames(this, "Add audio files", QStandardPaths::writableLocation(QStandardPaths::MusicLocation), "Audio Files (*.mp3 *.wav)");
    for (const QString &file : files) {
        ui->playlistWidget->addItem(file);
    }
}

QStringList QPlaylistEdit::getUpdatedPlaylist() const {
    QStringList result;
    for (int i = 0; i < ui->playlistWidget->count(); ++i) {
        result << ui->playlistWidget->item(i)->text();
    }
    return result;
}

QString QPlaylistEdit::getPlaylistName() const {
    return ui->titleEdit->text();
}

void QPlaylistEdit::om_removeButton_clicked() {
    auto selectedTracks = ui->playlistWidget->selectedItems();
    for (QListWidgetItem *track : selectedTracks) {
        delete ui->playlistWidget->takeItem(ui->playlistWidget->row(track));
    }
}
