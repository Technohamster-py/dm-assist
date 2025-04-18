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
    explicit QInitiativeTrackerWidget(QWidget *parent = nullptr);

//    ~QInitiativeTrackerWidget() override;

signals:
    void encounterLoaded();
    void currentEntityChanged(int currentIndex);
    void share(Encounter* encounter);

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
     * @brief Вычисляет арифметическое выражение в ячейке HP.
     * @param row Номер строки.
     * @param column Номер колонки (должна быть колонка HP).
     */
    void evaluateHP(int row, int column);

    /**
     * @brief Обработчик изменения содержимого ячеек.
     * @param row Номер строки.
     * @param column Номер колонки.
     */
    void handleCellChanged(int row, int column);

    /**
     * @brief Подсвечивает текущего активного персонажа.
     */
    void highlightCurrentRow();

//    void on_backButton_clicked();
//    void on_nextButton_clicked();
//    void on_shareButton_clicked();

private:
    Ui::QInitiativeTrackerWidget *ui;
    int currentRowIndex = 0;    ///< Индекс текущего активного персонажа.

    /**
     * @brief Вставляет новую строку с необязательными значениями по умолчанию.
     * @param defaultValues Список значений по умолчанию.
     */
    void insertRow(const QStringList &defaultValues = QStringList());

    /**
     * @brief Оценивает арифметическое выражение в строке.
     * @param expression Строка с выражением (например, "100-35").
     * @return Результат выражения или исходная строка, если ошибка.
     */
    QString evaluateExpression(const QString &expression);

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
