#ifndef DM_ASSIST_ABSTRACTCHARSHEETWIDGET_H
#define DM_ASSIST_ABSTRACTCHARSHEETWIDGET_H

#include <QDir>
#include <QFileInfo>
#include <QWidget>
#include <QRandomGenerator>
#include "../initiative-tracker/initiativetrackerwidget.h"

/**
 * @class AbstractCharsheetWidget
 * @brief An abstract base class for character sheet widgets in an application.
 *
 * The AbstractCharsheetWidget provides a foundation for creating widgets
 * that display or interact with character sheets. This class serves as a
 * template for implementing specific character sheet behavior and visualization.
 * Derived classes should implement the necessary virtual methods to define
 * widget-specific functionality.
 *
 * @details
 * This class is intended to represent UI components or functionality
 * related to a character sheet system, such as displaying attributes, equipment,
 * or other character-related data. Since it is an abstract class, it cannot
 * be instantiated directly and must be subclassed to provide concrete behavior.
 * This design enables flexibility and promotes reuse of common widget logic across
 * the application.
 *
 * Key features include:
 * - A standardized interface for character sheet-related components.
 * - Base functionality for derived widgets if provided in the implementation.
 * - Support for extending or customizing character sheet behaviors.
 */
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

    static QString campaignDirFromFile(const QString& filePath) {
        QFileInfo fi(filePath);
        QDir dir = fi.absoluteDir();
        dir.cdUp();
        return dir.absolutePath();
    };

public slots:
    virtual void updateTranslator() = 0;

signals:
    void fileLoaded();
    void rollRequested(const QString& rollCommand);

protected:
    QString m_originalFilePath;
    QString m_campaignPath = "";
};


#endif //DM_ASSIST_ABSTRACTCHARSHEETWIDGET_H
