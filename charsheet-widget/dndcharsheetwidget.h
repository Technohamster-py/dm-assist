#ifndef DM_ASSIST_DNDCHARSHEETWIDGET_H
#define DM_ASSIST_DNDCHARSHEETWIDGET_H

#include "abstractcharsheetwidget.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QCheckBox>
#include <QSpinBox>
#include <QFile>

QT_BEGIN_NAMESPACE
namespace Ui { class DndCharsheetWidget; }
QT_END_NAMESPACE

/**
 * @brief Represents a widget for managing and displaying Dungeons & Dragons character sheets.
 *
 * This widget provides functionality to load, save, and manipulate character sheets
 * for Dungeons & Dragons gameplay. It offers derived features such as interaction
 * with initiative trackers, dynamic UI updates, and handling of character-specific data.
 */
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

    static QString parseParagraphs(const QJsonArray& content);
};


#endif //DM_ASSIST_DNDCHARSHEETWIDGET_H
