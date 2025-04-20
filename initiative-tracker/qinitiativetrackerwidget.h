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
    /**
     * @brief Конструктор виджета.
     * @param parent Родительский виджет.
     */
    explicit QInitiativeTrackerWidget(QWidget *parent = nullptr, InitiativeModel *sharedModel = nullptr);

//    ~QInitiativeTrackerWidget() override;

signals:
    void encounterLoaded();
    void currentEntityChanged(int currentIndex);
//    void share(Encounter* encounter);

private slots:
    /**
     * @brief Добавляет новую строку в таблицу.
     */
    void addRow();

    /**
     * @brief Удаляет строку по нажатию кнопки.
     */
//    void deleteRow();

    /**
     * @brief Переходит к следующему активному персонажу.
     */
    void nextTurn();

    /**
     * @brief Сортирует таблицу по инициативе (по убыванию).
     */
    void sortTable();

    /**
     * @brief Открывает расшареное окно
     */
    void openSharedWindow();

//    void on_backButton_clicked();
//    void on_nextButton_clicked();
//    void on_shareButton_clicked();

private:
    Ui::QInitiativeTrackerWidget *ui;
    int currentRowIndex = 0;    ///< Индекс текущего активного персонажа.

    InitiativeModel *model;

    void setupUI();
};
#endif //DM_ASSIST_QINITIATIVETRACKERWIDGET_H
