#ifndef DM_ASSIST_QINITIATIVETRACKERWIDGET_H
#define DM_ASSIST_QINITIATIVETRACKERWIDGET_H

#include <QWidget>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QList>
#include <QPointer>
#include "initiativemodel.h"
#include <QJsonDocument>
#include <QJsonObject>



QT_BEGIN_NAMESPACE
namespace Ui { class QInitiativeTrackerWidget; class qDndInitiativeEntityEditWidget; class qPlayerInitiativeView; }
QT_END_NAMESPACE

/**
 * @brief A widget for managing and displaying an initiative tracker.
 *
 * This widget allows users to manage a list of characters in an initiative tracker
 * by adding, removing, sorting, and navigating through characters' turns. It provides
 * functionality to save, load, and append characters' data from files, as well as
 * control the visibility of specific columns and fields.
 */
class QInitiativeTrackerWidget : public QWidget {
Q_OBJECT


public:
    explicit QInitiativeTrackerWidget(QWidget *parent = nullptr, InitiativeModel *sharedModel = nullptr);

signals:
    void fieldVisibilityChanged(int field, bool hidden);
    void columnHidden(int  column);
    void columnShown(int column);

public slots:
    void loadFromFile(QString filename);
    void saveToFile(QString filename);

    void addFromFile(QString filename);

    void setSharedFieldVisible(int index, bool visible);
    void setHpDisplayMode(int mode);
    void setHpComboBoxVisible(int visible);

    void addCharacterFromJson(const QJsonDocument& characterDocument);



private slots:
    void addRow();
    void nextTurn();
    void prevTurn();
    void sortTable();
    void openSharedWindow();

    void on_saveButton_clicked();
    void on_loadButton_clicked();
    void on_addFromFileButton_clicked();

private:
    Ui::QInitiativeTrackerWidget *ui;
    int currentRowIndex = 0;    ///< Индекс текущего активного персонажа.

    int visibleColumns = 0;
    QList<QPointer<QWidget>> sharedWindows;

    InitiativeModel *model;

    void setupUI();
    void closeEvent(QCloseEvent *event);

    bool isSharedHpVisible = false;
    bool isSharedMaxHpVisible = false;
    bool isSharedAcVisible = true;
};
#endif //DM_ASSIST_QINITIATIVETRACKERWIDGET_H
