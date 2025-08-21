#ifndef DM_ASSIST_DNDBESTIARYPAGE_H
#define DM_ASSIST_DNDBESTIARYPAGE_H

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

    virtual void addToInitiative(InitiativeTrackerWidget *initiativeTrackerWidget, bool autoRoll = false){};

protected:
    virtual void loadFromFile(QString filePath) override;
    QMap<QString, QList<BestiaryItem>> descriptionSections;

private:
    Ui::DndBestiaryPage *ui;
};


#endif //DM_ASSIST_DNDBESTIARYPAGE_H
