//
// Created by arseniy on 27.03.2024.
//

#ifndef DM_ASSIST_QINITIATIVETRACKERWIDGET_H
#define DM_ASSIST_QINITIATIVETRACKERWIDGET_H

#include <QWidget>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include "encounter.h"
#include "initiativemodel.h"




QT_BEGIN_NAMESPACE
namespace Ui { class QInitiativeTrackerWidget; class qDndInitiativeEntityEditWidget; class qPlayerInitiativeView; }
QT_END_NAMESPACE

class QInitiativeTrackerWidget : public QWidget {
Q_OBJECT


public:
    explicit QInitiativeTrackerWidget(QWidget *parent = nullptr, InitiativeModel *sharedModel = nullptr);

public slots:
    void loadFromFile(QString filename);
    void saveToFile(QString filename);

    void loadSettings();
    void saveSettings();


private slots:
    void addRow();
    void nextTurn();
    void prevTurn();
    void sortTable();
    void openSharedWindow();

    void on_saveButton_clicked();
    void on_loadButton_clicked();

private:
    Ui::QInitiativeTrackerWidget *ui;
    int currentRowIndex = 0;    ///< Индекс текущего активного персонажа.

    int visibleColumns = 0;
    QList<QPointer<QWidget>> sharedWindows;

    InitiativeModel *model;

    void setupUI();
    void closeEvent(QCloseEvent *event);
};
#endif //DM_ASSIST_QINITIATIVETRACKERWIDGET_H
