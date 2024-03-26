//
// Created by arsen on 06.03.2024.
//

#ifndef DM_ASSIST_QPLAYER_H
#define DM_ASSIST_QPLAYER_H

#include <QWidget>
#include <QMediaPlayer>
#include <QMediaPlaylist>


QT_BEGIN_NAMESPACE
namespace Ui { class QPlayer; }
QT_END_NAMESPACE

class QPlayer : public QWidget {
Q_OBJECT

public:
    QPlayer(QWidget *parent, QString title, int numId);
    QPlayer(QWidget *parent, QString pathToXml);

    ~QPlayer() override;

    QString getPlaylistName() const{return m_playlistName;};
    void setPlsylistName(QString name);

    QMediaPlaylist *playlist; ///< Плейлист проигрывателя

signals:
    void playlistNameChanged();

protected:
    void loadFromXml(QString pathToXml);

private slots:
    void on_editButton_clicked();

private:
    Ui::QPlayer *ui;
    QString m_playlistName;
    int m_id;

    //QStandardItemModel *m_playlistModel; ///< Модель данных плейлиста для отображения
    QMediaPlayer *m_player; ///< Проигрыватель
};


#endif //DM_ASSIST_QPLAYER_H
