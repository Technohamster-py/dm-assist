#ifndef DM_ASSIST_DNDCHARSHEETWIDGET_H
#define DM_ASSIST_DNDCHARSHEETWIDGET_H

#include "abstractcharsheetwidget.h"
#include <QJsonDocument>
#include <QFile>

QT_BEGIN_NAMESPACE
namespace Ui { class DndCharsheetWidget; }
QT_END_NAMESPACE

class DndCharsheetWidget : public AbstractCharsheetWidget {
Q_OBJECT

public:
    explicit DndCharsheetWidget(QWidget* parent = nullptr);
    ~DndCharsheetWidget() override;

    void loadFromFile(QString path) override;
    void saveToFile(QString filePath = nullptr) override;

    void addToInitiative(QInitiativeTrackerWidget* initiativeTrackerWidget) override;


private:
    Ui::DndCharsheetWidget *ui;

    QJsonDocument m_originalDocument;
    QJsonObject m_dataObject;

    void populateWidget();
};


#endif //DM_ASSIST_DNDCHARSHEETWIDGET_H
