#ifndef DM_ASSIST_ABSTRACTCHARSHEETWIDGET_H
#define DM_ASSIST_ABSTRACTCHARSHEETWIDGET_H

#include <QDir>
#include <QFileInfo>
#include <QWidget>
#include <QRandomGenerator>
#include <QtNetwork>
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
    explicit AbstractCharsheetWidget(const QString& filePath, QWidget* parent = nullptr) : QWidget(parent) {};
    ~AbstractCharsheetWidget() = default;

    virtual void loadFromFile(const QString &path) = 0;
    virtual void saveToFile(QString filePath = nullptr) = 0;

    virtual void addToInitiative(InitiativeTrackerWidget *initiativeTrackerWidget, bool autoRoll = false) = 0;
    virtual void setTokenPixmap(const QString& filePath) = 0;

    static int rollDice(int diceValue){
        return QRandomGenerator::global()->bounded(1, diceValue);
    };

    static QString campaignDirFromFile(const QString& filePath) {
        QFileInfo fi(filePath);
        QDir dir = fi.absoluteDir();
        dir.cdUp();
        return dir.absolutePath();
    };

    static QString getTokenFileName(const QString& campaignPath, const QString& tokenUrl){
        QUrl qurl(tokenUrl);
        if (!qurl.isValid()) {
            return "";
        }

        QString filename = qurl.fileName();
        if (filename.isEmpty()) {
            return "";
        }

        QDir dir(campaignPath);
        if (!dir.exists()) {
            return "";
        }

        if (!dir.exists("Tokens")) {
            return "";
        }

        QString fullPath = dir.filePath("Tokens/" + filename);
        QFileInfo fi(fullPath);
        if (!fi.exists()) {
            return "";
        }

        return fullPath;
    }

public slots:
    virtual void updateTranslator() = 0;

signals:
    void fileLoaded();
    void rollRequested(const QString& rollCommand);

protected:
    QString m_originalFilePath;
    QString m_campaignPath = "";
    QNetworkAccessManager* m_manager;

    virtual bool downloadToken(const QString& link) {
        QUrl qurl(link);
        if (!qurl.isValid()) {
            qWarning() << "Invalid URL:" << link;
            return false;
        }

        QString filename = qurl.fileName();
        if (filename.isEmpty()) {
//        qWarning() << "URL does not contain filename:" << link;
            return false;
        }

        QDir dir(m_campaignPath);
        if (!dir.exists()) {
//        qWarning() << "Campaign dir does not exist:" << m_campaignPath;
            return false;
        }

        // ensure tokens/ folder
        if (!dir.exists("Tokens")) {
            if (!dir.mkdir("Tokens")) {
                qWarning() << "Cannot create tokens dir!";
                return false;
            }
        }

        QString fullPath = dir.filePath("Tokens/" + filename);
        QFileInfo fi(fullPath);
        if (fi.exists()) {
//        qInfo() << "Token already exists:" << fullPath;
            return false;
        }

        // async download
        QNetworkRequest req(qurl);
        auto reply = m_manager->get(req);

        connect(reply, &QNetworkReply::finished, this, [=]() {
            if (reply->error() != QNetworkReply::NoError) {
                qWarning() << "Download failed:" << reply->errorString();
                reply->deleteLater();
                return;
            }
            QByteArray data = reply->readAll();

            QFile f(fullPath);
            if (!f.open(QIODevice::WriteOnly)) {
                qWarning() << "Cannot write file:" << fullPath;
                reply->deleteLater();
                return;
            }
            f.write(data);
            f.close();
//        qInfo() << "Saved token:" << fullPath;
            reply->deleteLater();
        });
        return true;
    };
};


#endif //DM_ASSIST_ABSTRACTCHARSHEETWIDGET_H
