#ifndef DM_ASSIST_DNDCHARSHEETWIDGET_H
#define DM_ASSIST_DNDCHARSHEETWIDGET_H

#include "abstractcharsheetwidget.h"
#include "dndmodels.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QCheckBox>
#include <QShortcut>
#include <QSpinBox>
#include <QFile>
#include <QTextEdit>

QT_BEGIN_NAMESPACE
namespace Ui { class DndCharsheetWidget; }
QT_END_NAMESPACE

/**
 * @brief Represents a widget for managing and displaying Dungeons & Dragons character sheet.
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

    void loadFromFile(const QString &path) override;
    void saveToFile(QString path) override;

    void addToInitiative(InitiativeTrackerWidget *initiativeTrackerWidget, bool autoRoll = false) override;

    static int bonusFromStat(int statValue) {return (statValue >= 10) ? (statValue - 10) / 2 : (statValue - 11) / 2;};
    static QString bonusFromString(const QString& string);
    static int proficiencyByLevel(int level) {return level / 5 + 2;};
    static QJsonArray serializeHtmlToJson(const QString &html);
public slots:
    void updateTranslator() override;
private:
    Ui::DndCharsheetWidget *ui;

    QJsonDocument m_originalDocument;
    QJsonObject m_dataObject;

    void populateWidget();
    void connectSignals();
    void setupShortcuts();
    void updateCheckBox(QCheckBox* checkBox, QSpinBox* baseSpinBox);
    void updateCheckBoxes();

    static QString parseParagraphs(const QJsonArray& content);
    void parseNotes();
    static QTextEdit* getFocusedEdit();

    QShortcut* m_boldShortcut;
    QShortcut* m_italicShortcut;
    QShortcut* m_underlineShortcut;

    DndAttackModel* attackModel;
    DndResourceModel* resourceModel;

    QJsonObject collectData(const QString& filePath = nullptr);

    void closeEvent(QCloseEvent *event) override;
};


#endif //DM_ASSIST_DNDCHARSHEETWIDGET_H
