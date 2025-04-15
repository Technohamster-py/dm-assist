//
// Created by arseniy on 23.04.2024.
//

#ifndef DM_ASSIST_QPLAYERINITIATIVEVIEW_H
#define DM_ASSIST_QPLAYERINITIATIVEVIEW_H

#include "qinitiativetrackerwidget.h"


QT_BEGIN_NAMESPACE
namespace Ui { class qPlayerInitiativeView; }
QT_END_NAMESPACE

class qPlayerInitiativeView : public QWidget {
Q_OBJECT

public:
    explicit qPlayerInitiativeView(QInitiativeTrackerWidget *parentTracker = nullptr, QWidget *parent = nullptr);

    ~qPlayerInitiativeView() override;

    enum hpMode{
        none = 0,
        numbers = 1,
        condition = 2
    };

    void setParentTracker(QInitiativeTrackerWidget *tracker);
    void hpSetVisible(bool visible);
    void acSetVisible(bool visible);

    void loadEncounter(Encounter *encounter);
    void next();
    void previous();

private:
    Ui::qPlayerInitiativeView *ui;

    void selectRow(int row);

    Encounter *m_encounter;
    QInitiativeTrackerWidget* m_parentTracker;

    int m_currentIndex;
    int m_entityCount;
    int m_currentEntityIndex;

    QString getEntityStatus(int hp, int maxHp);

private slots:
    void changeActiveEntity(int index);
};


#endif //DM_ASSIST_QPLAYERINITIATIVEVIEW_H
