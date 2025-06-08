#ifndef DM_ASSIST_DNDCHARSHEETWIDGET_H
#define DM_ASSIST_DNDCHARSHEETWIDGET_H

#include "abstractcharsheetwidget.h"
#include <QJsonDocument>
#include <QCheckBox>
#include <QSpinBox>
#include <QFile>

QT_BEGIN_NAMESPACE
namespace Ui { class DndCharsheetWidget; }
QT_END_NAMESPACE

class DndCharsheetWidget : public AbstractCharsheetWidget {
Q_OBJECT

public:
    explicit DndCharsheetWidget(QWidget* parent = nullptr);
    DndCharsheetWidget(const QString& filePath, QWidget* parent = nullptr);
    ~DndCharsheetWidget() override;

    void loadFromFile(QString path) override;
    void saveToFile(QString filePath = nullptr) override;

    void addToInitiative(InitiativeTrackerWidget* initiativeTrackerWidget) override;

    static int bonusFromStat(int statValue) {return (statValue - 10) / 2;};
    static int proficiencyByLevel(int level) {return level / 5 + 2;};

private:
    Ui::DndCharsheetWidget *ui;

    QJsonDocument m_originalDocument;
    QJsonObject m_dataObject;

    void populateWidget();
    void connectSignals();
    void updateCheckBox(QCheckBox* checkBox, QSpinBox* baseSpinBox);
    void updateCheckBoxes();
};


#endif //DM_ASSIST_DNDCHARSHEETWIDGET_H
