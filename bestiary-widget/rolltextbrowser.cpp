#include "rolltextbrowser.h"

RollTextBrowser::RollTextBrowser(QWidget *parent) : QTextBrowser(parent) {
    setOpenLinks(false);
    connect(this, &QTextBrowser::anchorClicked, this, &RollTextBrowser::onAnchorClicked);
}

void RollTextBrowser::setCustomHtml(const QString &html) {
    QString processed = html;

    QRegularExpression re1(R"(\[\[\/r ([^\]]+)\]\]\s*\(([^)]+)\))");    ///< [[/r 1d20+16]] (+16)
    auto it = re1.globalMatch(html);
    while (it.hasNext()) {
        auto m = it.next();
        QString rollExpr = m.captured(1); ///< e.g. 1d20+5
        QString shownText = m.captured(2); ///< e.g. +5
        QString link = QString("<a href=\"roll: %1\">%2</a>").arg(rollExpr, shownText);
        processed.replace(m.captured(0), link);
    }

    QRegularExpression re2(R"(\[\[\/r ([^\]]+)\]\])");  ///< [[/r 1d20+16]
    it = re2.globalMatch(processed);
    while (it.hasNext()) {
        auto m = it.next();
        QString rollExpr = m.captured(1); ///< e.g. 1d20+5
        QString shownText = rollExpr;
        QString link = QString("<a href=\"roll: %1\">%2</a>").arg(rollExpr, shownText);
        processed.replace(m.captured(0), link);
    }

    QRegularExpression re3(R"(\{@hit ([+-]?\d+)\})"); ///< {@hit 12}
    it = re3.globalMatch(processed);
    while (it.hasNext()){
        auto m = it.next();
        QString mod = m.captured(1);
        QString rollExpr;
        if (!mod.startsWith("-") && !mod.startsWith("+"))
            rollExpr = "1d20 + " + mod;
        else
            rollExpr = "1d20 " + mod;
        QString link = QString("<a href=\"roll: %1\">%2</a>").arg(rollExpr, mod);
        processed.replace(m.captured(0), link);
    }

    QRegularExpression re4(R"((\d+)\(\{@damage (\d+d\d+ [+-]? \d+)\}\}\))"); ///< 36({@damage 5d12 + 4}})
    it = re4.globalMatch(processed);
    while (it.hasNext()){
        auto m = it.next();
        QString rollExpr = m.captured(2);
        QString shownText = m.captured(1);
        QString link = QString("<a href=\"roll: %1\">%2</a>").arg(rollExpr, shownText);
        processed.replace(m.captured(0), link);
    }


    setHtml(processed);
}

void RollTextBrowser::onAnchorClicked(const QUrl &url) {
    if (url.scheme() == "roll") {
        emit rollRequested(url.path());
    }
}


