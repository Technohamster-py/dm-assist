#include "qplayerwidget.h"
#include "ui_qplayer.h"
#include "ui_qplaylistedit.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
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
          playlistName(std::move(title)),
          playKey(nullptr)
{
    ui->setupUi(this);
    ui->titleLabel->setText(playlistName);
    ui->numberLabel->setText(QString::number(id));

    setAcceptDrops(true);

    localDir = QStandardPaths::writableLocation(QStandardPaths::MusicLocation) + QString("/dm_assist_files/playlists/tmp/").arg(id);
    QDir().mkpath(localDir);

    BASS_Free();

    if (!BASS_Init(BASS_DEVICE_INDEX, 44100, 0, nullptr, nullptr)) {
        auto err = BASS_ErrorGetCode();
        qDebug() << "BASS init error:" << err;
        QMessageBox::critical(this, "BASS Init Failed",
                              QString("Could not initialize BASS on selected device.\nError code: %1").arg(err));
        return;
    }

    connect(ui->playButton, &QPushButton::clicked, this, &QPlayer::on_playButton_clicked);
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

void QPlayer::setPlayShortcut(QString key) {
    if (playKey)
        delete playKey;
    playKey = new QShortcut(QKeySequence(key), this);
    connect(playKey, &QShortcut::activated, this, &QPlayer::playShortcutTriggered);
}

void QPlayer::playShortcutTriggered() {
    emit playerStarted(); // для внешнего управления, если нужно
    play();
}

void QPlayer::addMedia(const QStringList &files) {
    for (const QString &file : files) {
        QFileInfo info(file);
        qDebug() << file;
        QString dest = localDir + "/" + info.fileName();
        qDebug() << dest;

        QDir destDir(localDir);
        if (!destDir.exists())
            destDir.mkpath(".");

        if (QFile::exists(dest)) {
            QFile::remove(dest);
        }
        QFile::copy(file, dest);
        filePaths.append(dest);
    }

    emit localDirPathChanged();
}

void QPlayer::on_playButton_clicked() {
    play();
}

void QPlayer::on_stopButton_clicked() {
    stop();
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
    stop();         // Остановим текущий поток
    freeStreams();  // Освободим старые

    for (const QString &file : filePaths) {
        HSTREAM stream = BASS_StreamCreateFile(FALSE, file.toStdString().c_str(), 0, 0, BASS_SAMPLE_LOOP);
        if (stream) {
            streams.append(stream);
        }
    }

    if (!streams.isEmpty()) {
        BASS_ChannelPlay(streams.first(), FALSE);
        isActive = true;
    }
}

void QPlayer::stop() {
    for (HSTREAM stream : streams) {
        BASS_ChannelStop(stream);
    }
    isActive = false;
    emit playerStopped();
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
    if (deviceIndex < 0) return {};
    BASS_DEVICEINFO info;
    if (BASS_GetDeviceInfo(deviceIndex, &info))
        return QString::fromLocal8Bit(info.name);
    return {};
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
        qWarning() << "Audio device not found:" << deviceName;
        return;
    }

    // Если уже такой активен — ничего не делаем
    if (deviceIndex == found) return;

    stop();
    freeStreams();
    BASS_Free();

    if (!BASS_Init(found, 44100, 0, nullptr, nullptr)) {
        QMessageBox::critical(this, "BASS Init Failed", "Could not initialize BASS on selected device.");
        return;
    }

    deviceIndex = found;
    qDebug() << "Audio output set to:" << deviceName;
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
    QStringList files = QFileDialog::getOpenFileNames(this, "Add audio files", QString(), "Audio Files (*.mp3 *.wav)");
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
