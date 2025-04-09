#ifndef DM_ASSIST_QPLAYERWIDGET_H
#define DM_ASSIST_QPLAYERWIDGET_H

#include <QCoreApplication>
#include <QDialog>
#include <QDomDocument>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QListWidget>
#include <QShortcut>
#include <QStandardPaths>
#include <QWidget>
#include "lib/bass/bass.h"

QT_BEGIN_NAMESPACE
namespace Ui { class QPlayer; class QPlaylistEdit;}
QT_END_NAMESPACE

class QPlayer : public QWidget {
Q_OBJECT

public:
    explicit QPlayer(QWidget *parent = nullptr, int id = 0, QString title = "Playlist");
    ~QPlayer() override;

    QString getPlaylistName() const { return playlistName; }
    void setPlaylistName(const QString &name);

    void addMedia(const QStringList &files);
    void setLocalDirPath(QString localDirPath);
    QString getLocalDirPath() const { return localDir; }

    void setPlayShortcut(QString key);

    void setAudioOutput(const QString &deviceName);
    void setAudioOutput(int deviceIndex);
    QString currentDeviceName() const;
    QStringList availableAudioDevices() const;


    void playTrackAt(int index);


signals:
    void playerStarted();
    void playerStopped();
    void playlistNameChanged();
    void localDirPathChanged();

public slots:
    void play();
    void stop();
    void edit();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void on_playButton_clicked();
    void on_stopButton_clicked();
    void on_editButton_clicked();
    void playShortcutTriggered();

    void playNextTrack();


private:
    void freeStreams();

    Ui::QPlayer *ui;
    QShortcut *playKey;

    QString playlistName;
    QString localDir;
    int id;
    bool isActive = false;

    QList<QString> filePaths;    // Список треков
    QList<HSTREAM> streams;      // Потоки BASS

    int m_deviceIndex = -1;

    int currentTrackIndex = 0;

};



class QPlaylistEdit : public QDialog {
Q_OBJECT

public:
    explicit QPlaylistEdit(QWidget *parent = nullptr, const QStringList &tracks = {}, QString title = "title");
    ~QPlaylistEdit();

    QStringList getUpdatedPlaylist() const;
    QString getPlaylistName() const;

private slots:
    void on_addButton_clicked();

private:
    Ui::QPlaylistEdit *ui;
};

#endif // DM_ASSIST_QPLAYERWIDGET_H
