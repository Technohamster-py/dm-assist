#ifndef DM_ASSIST_ABSTRACTCHARSHEETWIDGET_H
#define DM_ASSIST_ABSTRACTCHARSHEETWIDGET_H

#include <QDir>
#include <QFileInfo>
#include <QWidget>
#include <QRandomGenerator>
#include "../initiative-tracker/initiativetrackerwidget.h"

class AbstractCharsheetWidget : public QWidget{
Q_OBJECT
public:
    AbstractCharsheetWidget(QWidget* parent = nullptr) : QWidget(parent) {};
    AbstractCharsheetWidget(const QString& filePath, QWidget* parent = nullptr) : QWidget(parent) {};
    ~AbstractCharsheetWidget() = default;

    virtual void loadFromFile(const QString &path) = 0;
    virtual void saveToFile(QString filePath = nullptr) = 0;

    virtual void addToInitiative(InitiativeTrackerWidget *initiativeTrackerWidget, bool autoRoll = false) = 0;

    static int rollDice(int diceValue){
        return QRandomGenerator::global()->bounded(1, diceValue);
    };

public slots:
    virtual void updateTranslator() = 0;

signals:
    void fileLoaded();
    void rollRequested(const QString& rollCommand);

protected:
    QString m_originalFilePath;
    QString m_campaignPath = "";
    virtual QString campaignDirFromFile(const QString& filePath) {
        QFileInfo fi(filePath);
        QDir dir = fi.absoluteDir();
        dir.cdUp();
        return dir.absolutePath();
    };
};


#endif //DM_ASSIST_ABSTRACTCHARSHEETWIDGET_H
