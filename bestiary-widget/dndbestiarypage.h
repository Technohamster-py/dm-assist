#ifndef DM_ASSIST_DNDBESTIARYPAGE_H
#define DM_ASSIST_DNDBESTIARYPAGE_H

#include <utility>


#include "../charsheet-widget/abstractcharsheetwidget.h"
#include "fvttparser.h"


QT_BEGIN_NAMESPACE
namespace Ui { class DndBestiaryPage; }
QT_END_NAMESPACE

class DndBestiaryPage : public AbstractCharsheetWidget {
Q_OBJECT

public:
    explicit DndBestiaryPage(QWidget *parent = nullptr);
    explicit DndBestiaryPage(const QString& filePath, QWidget* parent = nullptr);

    ~DndBestiaryPage() override;

    static int bonusFromStat(int statValue) {return (statValue >= 10) ? (statValue - 10) / 2 : (statValue - 11) / 2;};

    void loadFromFile(const QString &filePath) override;
    virtual void populateWidget(BestiaryPageData data);
    void saveToFile(QString filePath) override{};

    void addToInitiative(InitiativeTrackerWidget *initiativeTrackerWidget, bool autoRoll = false) override;
    void setTokenPixmap(const QString& filePath) override;

public slots:
    void updateTranslator() override;

protected:
    QMap<QString, QList<BestiaryItem>> descriptionSections;
    IFvttParser *m_parser = nullptr;

    bool downloadToken(const QString& link) override;

private:
    Ui::DndBestiaryPage *ui;
};


#endif //DM_ASSIST_DNDBESTIARYPAGE_H
