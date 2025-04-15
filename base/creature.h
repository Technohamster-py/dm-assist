//
// Created by akhomchenko on 11.04.24.
//

#ifndef DM_ASSIST_CREATURE_H
#define DM_ASSIST_CREATURE_H


#include <QDomDocument>
#include <QApplication>
#include <QFile>


class Creature : public QObject{
    Q_OBJECT

public:
    explicit Creature(QString creatureName);
    explicit Creature(QFile *xmlConfig);

    ~Creature();

    int loadCreatureFromFile(QFile *xmlConfigFile = nullptr);
    int saveCreatureToFile(QString pathToXml = QCoreApplication::applicationDirPath());

    QString getTitle() const {return title;};

    enum FileErrors{
        ErrorNone = 0,
        OpenFileError = -1,
        ParseFileError = -2
    };

signals:
    void titleChanged();
    void xmlLoadError(QString errString);
    void xmlSaveError(QString errString);

protected:
    int id;
    QString title;
    QDomDocument configDom;
};


#endif //DM_ASSIST_CREATURE_H
