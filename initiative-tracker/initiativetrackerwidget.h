#ifndef DM_ASSIST_INITIATIVETRACKERWIDGET_H
#define DM_ASSIST_INITIATIVETRACKERWIDGET_H

#include <QWidget>
#include <QSortFilterProxyModel>
#include <QStandardPaths>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QList>
#include <QPointer>
#include "initiativemodel.h"
#include <QJsonDocument>
#include <QJsonObject>



QT_BEGIN_NAMESPACE
namespace Ui { class InitiativeTrackerWidget; class qDndInitiativeEntityEditWidget; class qPlayerInitiativeView; }
QT_END_NAMESPACE

/**
 * @brief A widget for managing and displaying an initiative tracker.
 *
 * This widget allows users to manage a list of characters in an initiative tracker
 * by adding, removing, sorting, and navigating through characters' turns. It provides
 * functionality to save, load, and append characters' data from files, as well as
 * control the visibility of specific columns and fields.
 */
class InitiativeTrackerWidget : public QWidget {
Q_OBJECT


public:
    explicit InitiativeTrackerWidget(QWidget *parent = nullptr, InitiativeModel *sharedModel = nullptr);

    void setBaseDir(QString dirPath = "");
signals:
    void fieldVisibilityChanged(int field, bool hidden);
    void columnHidden(int  column);
    void columnShown(int column);

public slots:
    void loadFromFile(const QString& filename);
    void saveToFile(const QString& filename);

    void addFromFile(const QString& filename);

    void setSharedFieldVisible(int index, bool visible);
    void setHpDisplayMode(int mode);
    void setHpComboBoxVisible(int visible);

    void addCharacter(const QJsonDocument& characterDocument);
    void addCharacter(QString name, int maxHp, int ac = 10, int hp = 0, int initiative = 0);

protected:
    int m_currentRound = 1;
    virtual void setupHeaderStretchPolicy();

protected slots:
    void addRow();
    void nextTurn();
    void prevTurn();
    void sortTable();
    void openSharedWindow();

private slots:
    void on_saveButton_clicked();
    void on_loadButton_clicked();
    void on_addFromFileButton_clicked();
    void on_resetButton_clicked();

private:
    Ui::InitiativeTrackerWidget *ui;

    QList<QPointer<QWidget>> sharedWindows;

    InitiativeModel *model;

    void setupUI();
    void closeEvent(QCloseEvent *event) override;

    bool isSharedHpVisible = false;
    bool isSharedMaxHpVisible = false;
    bool isSharedAcVisible = true;

    QString m_baseDirectoryPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
};
#endif //DM_ASSIST_INITIATIVETRACKERWIDGET_H
