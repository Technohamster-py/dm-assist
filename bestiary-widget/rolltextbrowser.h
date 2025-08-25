#ifndef DM_ASSIST_ROLLTEXTBROWSER_H
#define DM_ASSIST_ROLLTEXTBROWSER_H

#include <QTextBrowser>
#include <QRegularExpression>

class RollTextBrowser : public QTextBrowser{
    Q_OBJECT
public:
    explicit RollTextBrowser(QWidget* parent = nullptr);
    void setCustomHtml(const QString& html);

signals:
    void rollRequested(const QString& rollExpr);
private slots:
    void onAnchorClicked(const QUrl &url);
};


#endif //DM_ASSIST_ROLLTEXTBROWSER_H
