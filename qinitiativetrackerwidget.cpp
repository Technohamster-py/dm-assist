//
// Created by arseniy on 27.03.2024.
//
#include <QItemSelectionModel>

#include "qinitiativetrackerwidget.h"

#include "ui_qinitiativetrackerwidget.h"
#include "ui_qdndinitiativeentityeditwidget.h"
#include "ui_qplayerinitiativeview.h"


#include <qdebug.h>

#include <QJSEngine>

/**
 * @brief Конструктор: инициализация интерфейса.
 */
QInitiativeTrackerWidget::QInitiativeTrackerWidget(QWidget *parent, InitiativeModel *sharedModel)
        : QWidget(parent), ui(new Ui::QInitiativeTrackerWidget)
{
    model = sharedModel ? sharedModel : new InitiativeModel(this);
    ui->setupUi(this);

    setupUI();
}

/**
 * @brief и настраивает интерфейс виджета.
 */
void QInitiativeTrackerWidget::setupUI() {
    ui->table->setModel(model);
    ui->table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->table->setDropIndicatorShown(true);

    connect(ui->addRowButton, &QPushButton::clicked, this, &QInitiativeTrackerWidget::addRow);
    connect(ui->nextButton, &QPushButton::clicked, this, &QInitiativeTrackerWidget::nextTurn);
    connect(ui->sortButton, &QPushButton::clicked, this, &QInitiativeTrackerWidget::sortTable);
    connect(ui->shareButton, &QPushButton::clicked, this, &QInitiativeTrackerWidget::openSharedWindow);

    connect(ui->table, &QTableView::clicked, this, [=](const QModelIndex &index) {
        if (index.column() == 5) {
            model->removeCharacter(index.row());
        }
    });
}

/**
 * @brief Добавляет новую строку в таблицу.
 */
void QInitiativeTrackerWidget::addRow() {
    InitiativeCharacter emptyCharacter;
    emptyCharacter.name = "New";
    emptyCharacter.initiative = 0;
    emptyCharacter.ac = 10;
    emptyCharacter.hp = "0";
    emptyCharacter.maxHp = 0;

    model->addCharacter(emptyCharacter);

    // Переводим фокус на новую строку для удобства
    int lastRow = model->rowCount() - 1;
    ui->table->selectRow(lastRow);
    ui->table->edit(model->index(lastRow, 0)); // сразу начинаем редактирование первой ячейки
}

void QInitiativeTrackerWidget::openSharedWindow() {
    // Создаем новое окно
    QWidget *sharedWindow = new QWidget;
    sharedWindow->setAttribute(Qt::WA_DeleteOnClose);
    sharedWindow->setWindowTitle("Shared Initiative Tracker");
    sharedWindow->resize(800, 400);

    // Используем тот же экземпляр модели
    QInitiativeTrackerWidget *sharedWidget = new QInitiativeTrackerWidget(nullptr, model);

    QVBoxLayout *layout = new QVBoxLayout(sharedWindow);
    layout->addWidget(sharedWidget);
    sharedWindow->setLayout(layout);
    sharedWindow->show();
}

/**
 * @brief Переходит к следующему ходу.
 */
void QInitiativeTrackerWidget::nextTurn() {
    int next = (model->getCurrentIndex() + 1) % model->rowCount();
    model->setCurrentIndex(next);
}

/**
 * @brief Сортирует таблицу по инициативе.
 */
void QInitiativeTrackerWidget::sortTable() {
    model->sortByInitiative();
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
