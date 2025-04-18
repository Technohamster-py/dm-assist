//
// Created by arseniy on 27.03.2024.
//
#include <QItemSelectionModel>

#include "qinitiativetrackerwidget.h"

#include "ui_qinitiativetrackerwidget.h"
#include "ui_qdndinitiativeentityeditwidget.h"
#include "ui_qplayerinitiativeview.h"


#include <qdebug.h>

#include "qInitiativeTrackerWidget.h"
#include <QDebug>
#include <QJSEngine>

/**
 * @brief Конструктор: инициализация интерфейса.
 */
QInitiativeTrackerWidget::QInitiativeTrackerWidget(QWidget *parent)
        : QWidget(parent), ui(new Ui::QInitiativeTrackerWidget)
{
    ui->setupUi(this);

    setupUI();
}

/**
 * @brief и настраивает интерфейс виджета.
 */
void QInitiativeTrackerWidget::setupUI() {
    ui->table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->table->setDropIndicatorShown(true);

    connect(ui->table, &QTableWidget::cellChanged, this, &QInitiativeTrackerWidget::handleCellChanged);


    connect(ui->addRowButton, &QPushButton::clicked, this, &QInitiativeTrackerWidget::addRow);
    connect(ui->nextButton, &QPushButton::clicked, this, &QInitiativeTrackerWidget::nextTurn);
    connect(ui->sortButton, &QPushButton::clicked, this, &QInitiativeTrackerWidget::sortTable);
}

/**
 * @brief Добавляет новую строку в таблицу.
 */
void QInitiativeTrackerWidget::addRow() {
    insertRow();
}

/**
 * @brief Вставляет строку и настраивает кнопки/ячейки.
 */
void QInitiativeTrackerWidget::insertRow(const QStringList &defaults) {
    int row = ui->table->rowCount();
    ui->table->insertRow(row);

    for (int col = 0; col < 5; ++col) {
        QTableWidgetItem *item = new QTableWidgetItem(defaults.value(col, ""));
        item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
        ui->table->setItem(row, col, item);
    }

    QPushButton *delButton = new QPushButton("X");
    connect(delButton, &QPushButton::clicked, this, [=]() {
        ui->table->removeRow(row);
        if (currentRowIndex >= ui->table->rowCount())
            currentRowIndex = 0;
        highlightCurrentRow();
    });
    ui->table->setCellWidget(row, 5, delButton);

    highlightCurrentRow();
}

/**
 * @brief Переходит к следующему ходу.
 */
void QInitiativeTrackerWidget::nextTurn() {
    if (ui->table->rowCount() == 0) return;
    currentRowIndex = (currentRowIndex + 1) % ui->table->rowCount();
    highlightCurrentRow();
}

/**
 * @brief Подсвечивает активную строку.
 */
void QInitiativeTrackerWidget::highlightCurrentRow() {
    for (int i = 0; i < ui->table->rowCount(); ++i) {
        for (int j = 0; j < ui->table->columnCount(); ++j) {
            QTableWidgetItem *item = ui->table->item(i, j);
            if (item)
                item->setBackground(i == currentRowIndex ? QColor(200, 230, 255) : Qt::white);
        }
    }
}

/**
 * @brief Сортирует таблицу по инициативе.
 */
void QInitiativeTrackerWidget::sortTable() {
    ui->table->sortItems(1, Qt::DescendingOrder);
    highlightCurrentRow();
}

/**
 * @brief Обрабатывает изменение содержимого ячейки.
 */
void QInitiativeTrackerWidget::handleCellChanged(int row, int column) {
    if (column == 3) { // HP
        evaluateHP(row, column);
    }
}

/**
 * @brief Вычисляет выражение в ячейке HP.
 */
void QInitiativeTrackerWidget::evaluateHP(int row, int column) {
    QString input = ui->table->item(row, column)->text();
    QString evaluated = evaluateExpression(input);
    ui->table->blockSignals(true);
    ui->table->item(row, column)->setText(evaluated);
    ui->table->blockSignals(false);
}

/**
 * @brief Пытается вычислить выражение через JS движок.
 * @param expression Строка с выражением.
 * @return Результат или исходная строка, если ошибка.
 */
QString QInitiativeTrackerWidget::evaluateExpression(const QString &expression) {
    QJSEngine engine;
    QJSValue result = engine.evaluate(expression);
    if (result.isNumber())
        return QString::number(result.toInt());
    return expression;
}


////////////////////////////////////////////////////
//           qPlayerInitiativeView                //
////////////////////////////////////////////////////

qPlayerInitiativeView::qPlayerInitiativeView(QInitiativeTrackerWidget *parentTracker, QWidget *parent) :
        QWidget(parent), ui(new Ui::qPlayerInitiativeView) {
    ui->setupUi(this);

//    loadEncounter(parentTracker->getEncounter());

    //connect(parentTracker, SIGNAL(currentEntityChanged), this, )
}

qPlayerInitiativeView::~qPlayerInitiativeView() {
    delete ui;
}

void qPlayerInitiativeView::loadEncounter(Encounter *encounter) {
    m_entityCount = encounter->getModel()->rowCount();
    m_encounter = encounter;

    CustomSortFilterProxyModel* initiativeProxyModel = new CustomSortFilterProxyModel();
    initiativeProxyModel->setSourceModel(encounter->getModel());
    initiativeProxyModel->setDynamicSortFilter(true);

    ui->encounterView->setModel(initiativeProxyModel);
    initiativeProxyModel->sort(0, Qt::AscendingOrder);

    ui->encounterView->setColumnWidth(0, 20);
    ui->encounterView->setColumnWidth(2, 20);
    ui->encounterView->setColumnWidth(3, 60);

    ui->encounterView->hideColumn(4);
    ui->encounterView->hideColumn(5);
    ui->encounterView->hideColumn(6);

    selectRow(0);
}

void qPlayerInitiativeView::selectRow(int row) {
    m_currentIndex = row;

    QItemSelectionModel *selectionModel = ui->encounterView->selectionModel();
    QItemSelection selection;
    QModelIndex topLeft = ui->encounterView->model()->index(row, 0);
    QModelIndex bottomRight = ui->encounterView->model()->index(row, 3);
    selection.select(topLeft, bottomRight);
    selectionModel->select(selection, QItemSelectionModel::ClearAndSelect);
}

void qPlayerInitiativeView::changeActiveEntity(int index) {
    selectRow(index);
}

void qPlayerInitiativeView::setParentTracker(QInitiativeTrackerWidget *tracker) {
    m_parentTracker = tracker;
    connect(m_parentTracker, SIGNAL(currentEntityChanged(int)), SLOT(changeActiveEntity(int)));
}
