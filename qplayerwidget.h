//
// Created by arsen on 06.03.2024.
//

#ifndef DM_ASSIST_QPLAYERWIDGET_H
#define DM_ASSIST_QPLAYERWIDGET_H

#include <QApplication>
#include <QDialog>
#include <QDomDocument>
#include <QFile>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QStandardItem>
#include <QStandardPaths>
#include <QShortcut>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class QPlayer; class QPlaylistEdit;}
QT_END_NAMESPACE

class QPlayer : public QWidget {
Q_OBJECT

public:
    QPlayer(QWidget *parent, QString title, int numId);
    QPlayer(QWidget *parent, QFile *xmlFile);
    QPlayer(QWidget *parent, int numId);

    ~QPlayer() override;

    [[nodiscard]] QString getPlaylistName() const{return playlistName;};
    void setPlaylistName(QString name);

    [[nodiscard]] int getId() const{return id;};
    [[nodiscard]] bool isPlaying() const {return isActive;};

    void saveToXml(QString pathToXml = QCoreApplication::applicationDirPath());
    void loadFromXml(QFile *xmlFile);
    void setPlayShortcut(QString key);

    QString getLocalDirPath() const {return localDir;};
    void setLocalDirPath(QString localDirPath);

    void addMedia(QStringList trackList);

    QMediaPlaylist *playlist; ///< Плейлист проигрывателя

signals:
    void playlistNameChanged();
    void localDirPathChanged();
    void playerStarted();
    void playerStopped();

public slots:
    void stop();
    void play();
    void edit();

protected:
    QString playlistName = "playlist";
    QString localDir = QStandardPaths::writableLocation(QStandardPaths::MusicLocation) + "dm_assis_files/playlists/";
    int id;
    bool isActive;

private slots:
    void on_editButton_clicked();
    void on_playButton_clicked();
    void playShortcutTriggered();

private:
    Ui::QPlayer *ui;
    QDomDocument xmlConfig;

    QShortcut *playKey;

    //QStandardItemModel *m_playlistModel; ///< Модель данных плейлиста для отображения
    QMediaPlayer *m_player; ///< Проигрыватель
};


class QPlaylistEdit : public QDialog {
Q_OBJECT

public:
    explicit QPlaylistEdit(QWidget *parent = nullptr, QPlayer *player = nullptr);

    void addMediaFiles(QStringList fileNames);

    ~QPlaylistEdit() override;

private slots:
    void on_addButton_clicked();
    void on_removeButton_clicked();
    void on_folderButton_clicked();
    void on_uploadButton_clicked();

    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::QPlaylistEdit *ui;
    QPlayer *m_player;
    QStandardItemModel *m_playlistModel;

    int m_selectedIndex;

    void displayPlaylist();
};

#endif //DM_ASSIST_QPLAYERWIDGET_H
