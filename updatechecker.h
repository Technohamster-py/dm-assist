#ifndef DM_ASSIST_UPDATECHECKER_H
#define DM_ASSIST_UPDATECHECKER_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVersionNumber>
#include <QWidget>
#include <utility>
#include <QLabel>

class UpdateChecker : public QObject{
    Q_OBJECT
public:
    UpdateChecker(const QString& currentVersion, const QString& repoUrl, QObject* parent = nullptr) : QObject(parent), m_currentVersionNumber(QVersionNumber::fromString(currentVersion)), m_repoUrl(repoUrl){
        m_manager = new QNetworkAccessManager(this);
        connect(m_manager, &QNetworkAccessManager::finished, this, &UpdateChecker::onReplyFinished);
    }

    void checkFotUpdates(){
        QNetworkRequest request(m_repoUrl);
        request.setHeader(QNetworkRequest::UserAgentHeader, "QtApp");
        m_manager->get(request);
    }

    QString latestVersion() const {return m_latestVersion;};
    QString latestUrl() const {return m_latestUrl;};

signals:
    void updateCheckFinished(bool updateAvailable);

private slots:
    void onReplyFinished(QNetworkReply* reply){
        if (reply->error()){
            reply->deleteLater();
            emit updateCheckFinished(false);
            return;
        }

        QJsonDocument jsonDocument = QJsonDocument::fromJson(reply->readAll());
        if (!jsonDocument.isObject()){
            reply->deleteLater();
            emit updateCheckFinished(false);
            return;
        }

        QJsonObject jsonObj = jsonDocument.object();
        m_latestVersion = jsonObj["tag_name"].toString().mid(1);
        m_latestUrl = jsonObj["html_url"].toString();
        QVersionNumber latestVersion = QVersionNumber::fromString(m_latestVersion);
        if (latestVersion > m_currentVersionNumber){
            emit updateCheckFinished(true);
        }
        else
            emit updateCheckFinished(false);

        reply->deleteLater();
    };

private:
    QVersionNumber m_currentVersionNumber;
    QString m_latestVersion;
    QString m_latestUrl;
    QNetworkAccessManager *m_manager;
    const QUrl m_repoUrl;
    bool hasUpdates = false;
};


class UpdateBannerWidget : public QWidget {
    Q_OBJECT
public:
    explicit UpdateBannerWidget(QWidget *parent, const QString &versionUrl = "https://github.com/Technohamster-py/dm-assist/releases/latest", const QString &currentVersion = "0.0",
                                const QString &latestVersion = "0.1");

    void setUrl(QString url) {m_versionUrl = std::move(url); updateLabel();};
    void setCurrentVersion(QString version) {m_currentVersion = std::move(version); updateLabel();};
    void setLatestVersion(QString version) {m_latestVersion = std::move(version); updateLabel();};
signals:
    void dismissed();
private slots:
    void onDismiss();
    void onOpenReleasePage();
    void updateLabel();
private:
    QString m_versionUrl;
    QString m_currentVersion;
    QString m_latestVersion;
    QLabel *label;
};

#endif //DM_ASSIST_UPDATECHECKER_H
