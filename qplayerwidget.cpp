#include "qplayerwidget.h"
#include "ui_qplayer.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QStandardPaths>


#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>

#include "bass/bass.h"

#define BASS_DEVICE_INDEX 1 // Можно сделать параметром, если нужно

QPlayer::QPlayer(QWidget *parent, int id, QString title)
        : QWidget(parent),
          ui(new Ui::QPlayer),
          id(id),
          playlistName(std::move(title)),
          playKey(nullptr)
{
    ui->setupUi(this);
    ui->titleLabel->setText(playlistName);

    setAcceptDrops(true);

    localDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QString("/playlist_%1/").arg(id);
    QDir().mkpath(localDir);

    if (!BASS_Init(BASS_DEVICE_INDEX, 44100, 0, nullptr, nullptr)) {
        QMessageBox::critical(this, "BASS", "Failed to initialize BASS for device.");
    }

    connect(ui->playButton, &QPushButton::clicked, this, &QPlayer::on_playButton_clicked);
    connect(ui->editButton, &QPushButton::clicked, this, &QPlayer::on_editButton_clicked);
}

QPlayer::~QPlayer() {
    stop();
    freeStreams();
    BASS_Free();
    delete ui;
}

void QPlayer::setPlaylistName(const QString &name) {
    playlistName = name;
    ui->titleLabel->setText(name);
    emit playlistNameChanged();
}

void QPlayer::setPlayShortcut(QString key) {
    if (playKey)
        delete playKey;

    playKey = new QShortcut(QKeySequence("Ctrl+" + key), this);
    connect(playKey, &QShortcut::activated, this, &QPlayer::playShortcutTriggered);
}

void QPlayer::playShortcutTriggered() {
    emit playerStarted(); // для внешнего управления, если нужно
    play();
}

void QPlayer::addMedia(const QStringList &files) {
    for (const QString &file : files) {
        QFileInfo info(file);
        QString dest = localDir + "/" + info.fileName();
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

void QPlayer::on_editButton_clicked() {
    edit();
}

void QPlayer::edit() {
    QPlaylistEdit editor(this, filePaths);
    if (editor.exec() == QDialog::Accepted) {
        QStringList newList = editor.getUpdatedPlaylist();

        // Обновим копии файлов, удалим старые
        QDir dir(localDir);
        for (const QFileInfo &file : dir.entryInfoList(QDir::Files)) {
            QFile::remove(file.absoluteFilePath());
        }

        filePaths.clear();
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
        ui->playButton->setText("Stop");
    }
}

void QPlayer::stop() {
    for (HSTREAM stream : streams) {
        BASS_ChannelStop(stream);
    }
    isActive = false;
    ui->playButton->setText("Play");
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



QPlaylistEdit::QPlaylistEdit(QWidget *parent, const QStringList &tracks)
        : QDialog(parent),
          ui(nullptr)
{
    setWindowTitle("Edit Playlist");
    resize(400, 300);

    QVBoxLayout *layout = new QVBoxLayout(this);

    listWidget = new QListWidget(this);
    listWidget->addItems(tracks);
    listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    listWidget->setDragDropMode(QAbstractItemView::InternalMove);

    QPushButton *addButton = new QPushButton("Add Files", this);
    QPushButton *okButton = new QPushButton("OK", this);

    layout->addWidget(listWidget);
    layout->addWidget(addButton);
    layout->addWidget(okButton);

    connect(addButton, &QPushButton::clicked, this, &QPlaylistEdit::on_addFilesButton_clicked);
    connect(okButton, &QPushButton::clicked, this, &QPlaylistEdit::accept);
}

QPlaylistEdit::~QPlaylistEdit() = default;

void QPlaylistEdit::on_addFilesButton_clicked() {
    QStringList files = QFileDialog::getOpenFileNames(this, "Add audio files", QString(), "Audio Files (*.mp3 *.wav)");
    for (const QString &file : files) {
        listWidget->addItem(file);
    }
}

QStringList QPlaylistEdit::getUpdatedPlaylist() const {
    QStringList result;
    for (int i = 0; i < listWidget->count(); ++i) {
        result << listWidget->item(i)->text();
    }
    return result;
}