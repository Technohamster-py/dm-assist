#ifndef DM_ASSIST_DNDBESTIARYPAGE_H
#define DM_ASSIST_DNDBESTIARYPAGE_H

#include <utility>
#include <QtNetwork>


#include "../charsheet-widget/abstractcharsheetwidget.h"
#include "fvttparser.h"


QT_BEGIN_NAMESPACE
namespace Ui { class DndBestiaryPage; }
QT_END_NAMESPACE

class DndBestiaryPage : public AbstractCharsheetWidget {
Q_OBJECT

public:
    explicit DndBestiaryPage(QWidget *parent = nullptr);
    explicit DndBestiaryPage(QString filePath, QWidget* parent = nullptr);

    ~DndBestiaryPage() override;

    static int bonusFromStat(int statValue) {return (statValue >= 10) ? (statValue - 10) / 2 : (statValue - 11) / 2;};

    virtual void loadFromFile(const QString &filePath) override;
    virtual void populateWidget(BestiaryPageData data);
    virtual void saveToFile(QString filePath = nullptr){};

    virtual void addToInitiative(InitiativeTrackerWidget *initiativeTrackerWidget, bool autoRoll = false);

public slots:
    void updateTranslator() override;

protected:
    QMap<QString, QList<BestiaryItem>> descriptionSections;
    IFvttParser *m_parser = nullptr;

    virtual bool downloadToken(const QString& link);
    QNetworkAccessManager* m_manager;

    virtual void setTokenPixmap(const QString& filePath);
private:
    Ui::DndBestiaryPage *ui;
    QString convertToHeader(QString type);

    void parseFromFvtt11(QJsonObject root);
    void parseFromFvtt10(QJsonObject root);
};


#endif //DM_ASSIST_DNDBESTIARYPAGE_H
