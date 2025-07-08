#ifndef DM_ASSIST_MUSICWIDGET_H
#define DM_ASSIST_MUSICWIDGET_H

#include <QCoreApplication>
#include <QDialog>
#include <QDomDocument>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QListWidget>
#include <QShortcut>
#include <QStandardPaths>
#include <QWidget>
#include "bass.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MusicPlayerWidget; class PlaylistEditDialog;}
QT_END_NAMESPACE

/**
 * @class QPlayer
 * @brief A widget-based audio player supporting playlist management and audio output configuration.
 *
 * The QPlayer class provides a user-friendly interface to manage and play audio playlists.
 * It supports functionalities such as adding media files, modifying volume, changing audio
 * output devices, and handling drag-and-drop operations for media files.
 */
class MusicPlayerWidget : public QWidget {
Q_OBJECT

public:
    explicit MusicPlayerWidget(QWidget *parent = nullptr, int id = 0, QString title = "Playlist");
    ~MusicPlayerWidget() override;

    QString getPlaylistName() const { return playlistName; }
    void setPlaylistName(const QString &name);
    int getPlaylistId() const {return id;}

    void setVolumeDivider(int value);
    int getVolumeDivider() const {return volumeDivider;};
    int volumeSliderPosition();

    void addMedia(const QStringList &files);
    void setLocalDirPath(const QString& localDirPath);
    QString getLocalDirPath() const { return localDir; }

    void setPlayShortcut(const QString& key);

    void setAudioOutput(const QString &deviceName);
    void setAudioOutput(int deviceIndex);
    QString currentDeviceName() const;
    static QStringList availableAudioDevices() ;


    void playTrackAt(int index);


signals:
    void playerStarted(int id);
    void playerStopped();
    void playlistNameChanged();
    void localDirPathChanged();

public slots:
    void play();
    void stop();
    void edit();
    void setVolume(int volume);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void on_playButton_clicked();
    void on_editButton_clicked();
    void on_muteButton_clicked();
    void playShortcutTriggered();

    void playNextTrack();
    void changeVolume(int volume) const;

private:
    void freeStreams();

    Ui::MusicPlayerWidget *ui;
    QShortcut *playKey;

    QString playlistName;
    QString localDir;
    int id;
    bool isActive = false;
    bool isMuted = false;
    int prevVolume = 100;

    QList<QString> filePaths;    // Список треков
    QList<HSTREAM> streams;      // Потоки BASS

    int m_deviceIndex = -1;

    int currentTrackIndex = 0;
    HSTREAM stream;
    int volumeDivider = 100;
};



class PlaylistEditDialog : public QDialog {
Q_OBJECT

public:
    explicit PlaylistEditDialog(QWidget *parent = nullptr, const QStringList &tracks = {}, const QString& title = "title");
    ~PlaylistEditDialog();

    QStringList getUpdatedPlaylist() const;
    QString getPlaylistName() const;

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void on_addButton_clicked();
    void om_removeButton_clicked();

private:
    Ui::PlaylistEditDialog *ui;
};

#endif // DM_ASSIST_MUSICWIDGET_H
