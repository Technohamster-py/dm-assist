#ifndef DM_ASSIST_DNDBESTIARYPAGE_H
#define DM_ASSIST_DNDBESTIARYPAGE_H

#include <utility>
#include <QtNetwork>


#include "../charsheet-widget/abstractcharsheetwidget.h"
#include "dndbeaststructure.h"


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

    virtual void saveToFile(QString filePath = nullptr){};

    virtual void addToInitiative(InitiativeTrackerWidget *initiativeTrackerWidget, bool autoRoll = false);


public slots:
    void updateTranslator() override;

protected:
    virtual void loadFromFile(const QString &filePath) override;
    QMap<QString, QList<BestiaryItem>> descriptionSections;

    QMap<QString, QString> typeToHeader {
            {"action", tr("Actions")},
            {"feat", tr("Feats")},
            {"lair", tr("Lair actions")},
            {"legendary", tr("Legendary actions")},
            {"spell", tr("Spells")},
            {"trait", tr("Traits")}
    };

    QMap<QString, QString> skillTitles {
            {"acr", tr("Acrobatics")},
            {"ani", tr("Animal handling")},
            {"arc", tr("Arcana")},
            {"ath", tr("Athletics")},
            {"dec", tr("Deception")},
            {"his", tr("History")},
            {"ins", tr("Insight")},
            {"itm", tr("Intimidation")},
            {"inv", tr("Investigation")},
            {"med", tr("Medicine")},
            {"nat", tr("Nature")},
            {"prc", tr("Perception")},
            {"prf", tr("Performance")},
            {"per", tr("Persuasion")},
            {"rel", tr("Religion")},
            {"slt", tr("Sleight of hand")},
            {"ste", tr("Stealth")},
            {"sur", tr("Survival")}
    };

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
