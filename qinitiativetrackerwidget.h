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



class CustomSortFilterProxyModel : public QSortFilterProxyModel
{
public:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override
    {
        QVariant leftData = sourceModel()->data(left);
        QVariant rightData = sourceModel()->data(right);

        if (leftData.isValid() && rightData.isValid()) {
            return leftData.toDouble() > rightData.toDouble();
        }

        return QSortFilterProxyModel::lessThan(left, right);
    }
};


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
#endif //DM_ASSIST_QINITIATIVETRACKERWIDGET_H
