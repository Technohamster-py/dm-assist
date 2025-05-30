#include "musicwidget.h"
#include "ui_musicplayer.h"
#include "ui_playlisteditdialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QPushButton>

#include <QRegularExpression>
#include <QRegularExpressionValidator>

#include "bass.h"

#define BASS_DEVICE_INDEX 1


////////////////////////////////////////////////
/////////       QPlayer                  ///////
////////////////////////////////////////////////

/**
 * @brief Constructor for the QPlayer class.
 *
 * Initializes a QPlayer widget, setting up UI components and environment for playback functionality.
 *
 * @param parent Pointer to the parent widget. Defaults to nullptr if no parent is provided.
 * @param id Unique identifier for the player instance.
 * @param title String representing the playlist name to be associated with this player.
 *
 * The constructor sets up the UI components for the player, initializes values for member variables,
 * and configures the shortcut key functionality and volume controls. Additionally, it sets the local
 * directory path for temporary playlist files and initializes the BASS library for audio handling.
 * If BASS initialization fails, an error message box is displayed with the corresponding error code.
 *
 * @note The constructor also sets up drag-and-drop functionality for the player widget.
 */
MusicPlayerWidget::MusicPlayerWidget(QWidget *parent, int id, QString title)
        : QWidget(parent),
          ui(new Ui::MusicPlayerWidget),
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

    connect(playKey, SIGNAL(activated()), this, SLOT(playShortcutTriggered()));
    connect(ui->volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(changeVolume(int)));

    ui->volumeSlider->setValue(100);
}

/**
 * @brief Destructor for the QPlayer class.
 *
 * Releases resources used by the QPlayer object. This includes:
 * - Stopping audio playback and emitting relevant signals.
 * - Freeing all audio streams, clearing the internal list of streams.
 * - Releasing the BASS audio library resources.
 * - Cleaning up the user interface associated with the QPlayer object.
 *
 * Ensures the proper cleanup of objects and resources to prevent memory leaks or undefined behavior
 * when the QPlayer instance is destroyed.
 */
MusicPlayerWidget::~MusicPlayerWidget() {
    stop();
    freeStreams();
    BASS_Free();
    delete ui;
}

/**
 * Updates the name of the playlist and performs essential state changes.
 *
 * This method sets a new name for the playlist if it differs from the current name.
 * It updates the UI component displaying the playlist name, renames the directory
 * associated with the playlist, and adjusts internal media file paths accordingly.
 * The function also emits a signal to notify observers about the change in the playlist name.
 *
 * Logic:
 * - Compares the input name with the current `playlistName`. If it is different:
 *     - Updates the `playlistName` member variable.
 *     - Updates the UI element `ui->titleLabel` to reflect the new name.
 *     - Renames the local directory (based on `localDir`) to match the new playlist name.
 *     - Adjusts internal media file paths (`filePaths`) by clearing the current list and re-adding media
 *       from the renamed directory using the `addMedia` method.
 *     - Updates the `localDir` to the new directory path.
 *     - Emits the `playlistNameChanged` signal to notify connected slots.
 *
 * Parameters:
 * @param name The new name to be assigned to the playlist. If this matches the current
 *             playlist name, the method does nothing.
 *
 * Signals:
 * - Emits `playlistNameChanged()` if the playlist name is successfully updated.
 */
void MusicPlayerWidget::setPlaylistName(const QString &name) {
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

/**
 * Sets the volume divider for the player.
 *
 * This method adjusts the volume divider to a value between 0 and 100. Any value
 * greater than 100 will be set to 100, and any value less than 0 will be set to 0.
 * After setting the volume divider, the current volume is recalculated and applied
 * based on the current position of the volume slider.
 *
 * @param value The desired volume divider value.
 */
void MusicPlayerWidget::setVolumeDivider(int value) {
    volumeDivider = value;
    if (volumeDivider > 100)
        volumeDivider = 100;
    if (volumeDivider < 0)
        volumeDivider = 0;
    changeVolume(ui->volumeSlider->value());
}

/**
 * @brief Sets the keyboard shortcut for triggering the play action.
 *
 * This function assigns a specified key sequence to the `playKey` shortcut,
 * which can be used to control the play action for the player.
 *
 * @param key The keyboard shortcut to be assigned. It should be a string representing
 *            the desired key sequence.
 */
void MusicPlayerWidget::setPlayShortcut(QString key) {
    playKey->setKey(key);
}

void MusicPlayerWidget::playShortcutTriggered() {
    play();
}

/**
 * @brief Adds media files to the local directory and updates the file paths.
 *
 * This function takes a list of file paths, copies each file to the specified
 * local directory, and appends the new location to the file paths list. If the
 * local directory does not exist, it creates the directory before copying the files.
 * After successfully adding the files, the function emits the localDirPathChanged signal
 * to indicate that the local directory path has been updated.
 *
 * @param files List of file paths to add to the local directory.
 *
 * @details
 * - Each file in the provided list is processed individually.
 * - The function checks if the local directory exists; if not, it creates the directory.
 * - Files are copied from their original location to the destination directory (localDir).
 * - The destination path is appended to the `filePaths` list.
 * - After completing the operation, the `localDirPathChanged` signal is emitted to notify
 *   listeners about the update.
 */
void MusicPlayerWidget::addMedia(const QStringList &files) {
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

void MusicPlayerWidget::on_playButton_clicked() {
    if (isActive)
        stop();
    else
        play();
}

void MusicPlayerWidget::on_editButton_clicked() {
    edit();
}

/**
 * @brief Opens the playlist editor, applies user modifications, and updates the playlist files.
 *
 * The `edit` function provides functionality to edit the current playlist. This involves:
 * - Opening a `PlaylistEditDialog` dialog to allow the user to modify the playlist name and content.
 * - If the dialog is accepted (user confirms changes):
 *   - Retrieving the updated playlist content and new playlist name from the editor.
 *   - Removing all existing files in the local directory (`localDir`) to prepare for the updated playlist.
 *   - Updating the `filePaths` list to be empty before adding new media.
 *   - Setting the new playlist name via the `setPlaylistName` method.
 *   - Adding the updated list of media files using the `addMedia` method. This step ensures the updated media is stored in the local directory and properly integrated into the application.
 *
 * If the user cancels the `PlaylistEditDialog` dialog, no changes are made to the playlist.
 */
void MusicPlayerWidget::edit() {
    PlaylistEditDialog editor(this, filePaths, playlistName);
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

/**
 * @brief Starts audio playback for the current playlist.
 *
 * This function initiates the audio playback process for the current playlist.
 * It emits the playerStarted signal with the player's ID, clears any currently active audio streams,
 * and marks the player as active. If there are no files in the playlist, the function returns early.
 * For each file in the playlist, it creates an audio stream using the BASS library and adds it
 * to the list of active streams. Finally, it starts playing the first track in the playlist by
 * calling playTrackAt with the index of the first track.
 *
 * @note The BASS_StreamCreateFile function is used to create audio streams for each file in the playlist.
 * @note If the playlist is empty, no playback will be initiated.
 * @note The function emits the playerStarted signal with the player ID at the start of execution.
 */
void MusicPlayerWidget::play() {
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

/**
 * @brief Plays a specific track in the playlist based on its index.
 *
 * The method stops the currently playing track (if any),
 * sets the specified track in the playlist to play, and updates the UI
 * to reflect the active playback state. Additionally, a synchronization
 * event is set to automatically play the next track when the current one ends.
 *
 * @param index The index of the track in the playlist to be played.
 *              Must be within the valid range of the playlist indices.
 */
void MusicPlayerWidget::playTrackAt(int index) {
    if (index < 0 || index >= streams.size()) return;

    stop(); // на всякий случай

    stream = streams[index];
    BASS_ChannelPlay(stream, FALSE);

    // Установка синхронизации на окончание трека
    BASS_ChannelSetSync(stream, BASS_SYNC_END, 0, [](HSYNC, DWORD handle, DWORD, void *user) {
        MusicPlayerWidget *self = static_cast<MusicPlayerWidget*>(user);
        QMetaObject::invokeMethod(self, "playNextTrack", Qt::QueuedConnection);
    }, this);

    isActive = true;
    ui->playButton->setStyleSheet("background-color: #a5bfda;");
    ui->playButton->setIcon(QIcon(":/player/stop.svg"));
}

/**
 * @brief Plays the next track in the playlist, cycling back to the first track if the end of the list is reached.
 *
 * This method increments the track index to play the next audio stream in the playlist.
 * If the current track index exceeds the available tracks, it wraps back to the first track
 * in the playlist. The playback functionality is handled by the playTrackAt method.
 *
 * Behavior:
 * - If the current track index surpasses the last track, it resets to zero to loop back
 *   to the beginning of the playlist.
 * - The playback control ensures seamless transition between tracks in the playlist.
 */
void MusicPlayerWidget::playNextTrack() {
    ++currentTrackIndex;

    if (currentTrackIndex >= streams.size()) {
        currentTrackIndex = 0; // или stop(), если не нужно зацикливать
    }

    playTrackAt(currentTrackIndex);
}

/**
 * @brief Stops audio playback and resets the player state.
 *
 * This function iterates through all active audio streams and stops them using the BASS library.
 * It then updates the player's state to inactive, emits a signal indicating playback has stopped,
 * and modifies the UI to reflect the stopped state (e.g., changing the play button's style and icon).
 */
void MusicPlayerWidget::stop() {
    for (HSTREAM stream : streams) {
        BASS_ChannelStop(stream);
    }
    isActive = false;
    emit playerStopped();
    ui->playButton->setStyleSheet("background: transparent;");
    ui->playButton->setIcon(QIcon(":/player/play.svg"));
}

/**
 * @brief Frees all audio streams managed by the player and clears the stream list.
 *
 * This function iterates through the list of audio streams (`streams`) and releases
 * the resources associated with each audio stream using the `BASS_StreamFree` method.
 * After freeing all streams, the list is cleared to ensure no invalid handles remain.
 *
 * This is a cleanup function primarily used to release resources when the audio
 * streams are no longer needed, ensuring proper resource management and avoiding
 * leaks.
 */
void MusicPlayerWidget::freeStreams() {
    for (HSTREAM stream : streams) {
        BASS_StreamFree(stream);
    }
    streams.clear();
}

/**
 * @brief Handles the drag-and-drop enter event for the QPlayer widget.
 *
 * This function is triggered when a drag-and-drop action enters the QPlayer widget.
 * If the dragged data contains URLs, the drag action is accepted to enable dropping files
 * onto the widget.
 *
 * @param event A pointer to the QDragEnterEvent object that contains details about
 * the drag action.
 */
void MusicPlayerWidget::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

/**
 * @brief Handles drop events and adds valid audio files to the playlist.
 *
 * This function processes a drop event by extracting the file URLs from the
 * MIME data. It then filters the files to ensure they exist, are actual files,
 * and have an audio file extension ("mp3" or "wav"). Valid files are added
 * to the media playlist using the `addMedia` function.
 *
 * @param event Pointer to the QDropEvent containing the dropped data.
 *
 * @details
 * - The function retrieves the URLs from the `mimeData` of the drop event.
 * - It verifies each file to ensure it exists, is not a directory, and
 *   has an acceptable audio file extension.
 * - Supported file formats are "mp3" and "wav".
 * - After filtering the valid files, the `addMedia` method is called to add
 *   them to the playlist and update the internal file paths.
 * - No actions are taken if no valid files are found.
 */
void MusicPlayerWidget::dropEvent(QDropEvent *event) {
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
 * @brief Sets the local directory path for the player and updates the local directory.
 *
 * This function constructs the full path for the local directory by appending the current
 * playlist name to the provided base path. It then emits the `localDirPathChanged()`
 * signal to notify listeners that the local directory path has been updated.
 *
 * @param localDirPath The base path to be set as the local directory.
 */
void MusicPlayerWidget::setLocalDirPath(QString localDirPath) {
    localDir = localDirPath + playlistName;
    emit localDirPathChanged();
}

/**
 * @brief Retrieves the list of available audio output devices.
 *
 * This method queries the system to find all audio output devices that are currently enabled
 * and available for use. It uses the BASS library to obtain information about the devices.
 *
 * @return QStringList A list of names of available audio devices.
 *
 * The returned list contains the names of all detected audio devices that are
 * flagged as enabled. Devices are queried using the `BASS_GetDeviceInfo` API
 * with a sequentially increasing device index starting from 1. If a device is
 * enabled (denoted by the `BASS_DEVICE_ENABLED` flag in the device info), its
 * name is included in the list.
 */
QStringList MusicPlayerWidget::availableAudioDevices() const {
    QStringList list;
    BASS_DEVICEINFO info;
    for (int i = 1; BASS_GetDeviceInfo(i, &info); i++) {
        if (info.flags & BASS_DEVICE_ENABLED)
            list << QString::fromLocal8Bit(info.name);
    }
    return list;
}

/**
 * @brief Retrieves the name of the current audio output device.
 *
 * This function returns the name of the currently active audio output device
 * used by the player. If no device is currently selected or if an error occurs while
 * retrieving the device information, an empty QString is returned.
 *
 * @return The name of the current audio output device as a QString, or an empty
 *         QString if no device is selected or an error occurs.
 */
QString MusicPlayerWidget::currentDeviceName() const {
    if (m_deviceIndex < 0) return {};
    BASS_DEVICEINFO info;
    if (BASS_GetDeviceInfo(m_deviceIndex, &info))
        return QString::fromLocal8Bit(info.name);
    return {};
}

/**
 * @brief Adjusts the volume level of the audio stream.
 *
 * This function modifies the audio stream's volume by scaling the input volume
 * and applying additional adjustments defined by the `volumeDivider`. The input volume
 * is normalized to a range of 0.0 to 1.0 and clamped if it exceeds the acceptable range.
 * The resulting volume is then passed to the BASS library to update the audio stream's attribute.
 *
 * @param volume An integer value representing the target volume level, typically in the range of 0 to 100.
 */
void MusicPlayerWidget::changeVolume(int volume) {

    float trueVolume = volume / 100.0f;

    if (trueVolume > 1.0f)
        trueVolume = 1.0f;
    if (trueVolume < 0.0f)
        trueVolume = 0.0f;

    BASS_ChannelSetAttribute(stream, BASS_ATTRIB_VOL, (trueVolume * static_cast<float>(volumeDivider) / 100.0f));
}

/**
 * @brief Sets the audio output device for the player based on the provided device name.
 *
 * This function attempts to find an audio output device with the given `deviceName` from the list
 * of devices retrieved via the BASS library. If the device is found and is enabled, it initializes
 * the BASS library for the selected device. If the device is currently active, no action is taken.
 *
 * The following operations are performed:
 * - Iterates through available audio devices using `BASS_GetDeviceInfo` and compares their names with the provided `deviceName`.
 * - Performs a check to ensure the device is enabled (`BASS_DEVICE_ENABLED` flag).
 * - If the device is found:
 *   - Stops any ongoing playback via `stop()`.
 *   - Frees all existing audio streams via `freeStreams()`.
 *   - Releases the previously initialized audio device using `BASS_Free()`.
 *   - Attempts to initialize the selected device with `BASS_Init`. If initialization fails, an error message is displayed using `QMessageBox::critical`.
 *   - Updates the `m_deviceIndex` member variable to the selected device index.
 *
 * @param deviceName The name of the audio output device to set.
 *
 * @note If the provided device name does not match any available device or if the device cannot be initialized,
 * the function performs no changes to the playback device.
 * @note The function does not activate a device if its corresponding `m_deviceIndex` is already set to the target.
 * @note The audio output is configured with a fixed sample rate of 44.1 kHz.
 */
void MusicPlayerWidget::setAudioOutput(const QString &deviceName) {
    BASS_DEVICEINFO info;
    int found = -1;

    for (int i = 1; BASS_GetDeviceInfo(i, &info); i++) {
        if (QString::fromLocal8Bit(info.name) == deviceName && (info.flags & BASS_DEVICE_ENABLED)) {
            found = i;
            break;
        }
    }
    if (found == -1) return;

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

/**
 * @brief Sets the audio output device by its index and reinitializes the audio system.
 *
 * This method configures the player to use a specified audio output device by its index.
 * If the given device index is already active or invalid (-1), no action is taken. If the
 * new device index differs from the currently active one, it stops the audio, frees
 * existing audio streams, and releases BASS resources. It then initializes BASS on the
 * specified device. If BASS initialization fails on the selected device, a critical
 * error message is displayed to inform the user.
 *
 * @param deviceIndex The index of the audio output device to set. If -1, no changes are made.
 */
void MusicPlayerWidget::setAudioOutput(int deviceIndex) {
    if (deviceIndex == -1) {
        return;
    }

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
void MusicPlayerWidget::setVolume(int volume) {
    if (volume > 100)
        volume = 100;
    if (volume < 0)
        volume = 0;
    ui->volumeSlider->setValue(volume);
}

////////////////////////////////////////////////
/////////       PlaylistEditDialog            ///////
////////////////////////////////////////////////

/**
 * @class PlaylistEditDialog
 * @brief A dialog for editing a playlist with features for reordering, adding, and removing tracks.
 *
 * PlaylistEditDialog is a QDialog subclass that provides a user interface to configure and modify
 * details of a playlist, such as its title and track order. It supports drag-and-drop for
 * reordering tracks and validates the playlist title.
 *
 * @param parent The parent QWidget of this dialog.
 * @param tracks A QStringList containing the initial list of tracks to be displayed in the dialog.
 * @param title The initial title of the playlist.
 */
PlaylistEditDialog::PlaylistEditDialog(QWidget *parent, const QStringList &tracks, QString title)
        : QDialog(parent), ui(new Ui::PlaylistEditDialog)
{
    ui->setupUi(this);
    resize(400, 300);

    QRegularExpression regex("[A-Za-z0-9\\-_ ]+");
    QRegularExpressionValidator *validator = new QRegularExpressionValidator(regex, ui->titleEdit);
    ui->titleEdit->setValidator(validator);

    ui->titleEdit->setText(title);
    ui->playlistWidget->addItems(tracks);
    ui->playlistWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->playlistWidget->setDragDropMode(QAbstractItemView::InternalMove);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &PlaylistEditDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &PlaylistEditDialog::reject);
}

PlaylistEditDialog::~PlaylistEditDialog() = default;

/**
 * @brief Slot triggered when the "Add" button is clicked.
 *
 * Opens a file dialog to allow the user to select audio files to add to the playlist.
 * The selected files are then added to the playlist widget for further use or manipulation.
 *
 * The file dialog filters for specific audio file formats, such as MP3 and WAV.
 * Files selected by the user from the dialog are appended to the playlist widget.
 */
void PlaylistEditDialog::on_addButton_clicked() {
    QStringList files = QFileDialog::getOpenFileNames(this, "Add audio files", QStandardPaths::writableLocation(QStandardPaths::MusicLocation), "Audio Files (*.mp3 *.wav)");
    for (const QString &file : files) {
        ui->playlistWidget->addItem(file);
    }
}

/**
 * @brief Retrieves the updated playlist from the UI.
 *
 * This function constructs a QStringList containing the text of all items
 * currently present in the playlist widget.
 *
 * @return A QStringList containing the text of all items in the playlist widget.
 */
QStringList PlaylistEditDialog::getUpdatedPlaylist() const {
    QStringList result;
    for (int i = 0; i < ui->playlistWidget->count(); ++i) {
        result << ui->playlistWidget->item(i)->text();
    }
    return result;
}

/**
 * @brief Retrieves the name of the playlist.
 *
 * This function accesses the user interface's titleEdit field to fetch
 * the current text, which represents the name of the playlist.
 *
 * @return The name of the playlist as a QString.
 */
QString PlaylistEditDialog::getPlaylistName() const {
    return ui->titleEdit->text();
}

/**
 * @brief Handles the removal of selected tracks from the playlist.
 *
 * This function is invoked when the remove button in the playlist editor is clicked.
 * It retrieves the selected tracks from the `playlistWidget`, deletes the corresponding
 * items, and removes them from the widget. This ensures that the selected tracks are
 * no longer part of the playlist view.
 *
 * @note The memory associated with the removed items is cleaned up using `delete`.
 */
void PlaylistEditDialog::om_removeButton_clicked() {
    auto selectedTracks = ui->playlistWidget->selectedItems();
    for (QListWidgetItem *track : selectedTracks) {
        delete ui->playlistWidget->takeItem(ui->playlistWidget->row(track));
    }
}
