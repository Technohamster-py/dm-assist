#ifndef DM_ASSIST_ABSTRACTCHARSHEETWIDGET_H
#define DM_ASSIST_ABSTRACTCHARSHEETWIDGET_H

#include <QWidget>
#include <QRandomGenerator>
#include "../initiative-tracker/initiativetrackerwidget.h"

class AbstractCharsheetWidget : public QWidget{
Q_OBJECT
public:
    AbstractCharsheetWidget(QWidget* parent = nullptr) : QWidget(parent) {};
    AbstractCharsheetWidget(const QString& filePath, QWidget* parent = nullptr) : QWidget(parent) {};
    ~AbstractCharsheetWidget() = default;

    virtual void loadFromFile(QString path) = 0;
    virtual void saveToFile(QString filePath = nullptr) = 0;

    virtual void addToInitiative(InitiativeTrackerWidget *initiativeTrackerWidget, bool autoRoll = false) = 0;

    static int rollDice(int diceValue){
        return QRandomGenerator::global()->bounded(1, diceValue);
    };

public slots:
    virtual void updateTranslator() = 0;

signals:
    void fileLoaded();
    void rollRequested(QString rollCommand);

protected:
    QString m_originalFilePath;
};


#endif //DM_ASSIST_ABSTRACTCHARSHEETWIDGET_H
