//
// Created by arseniy on 23.04.2024.
//

// You may need to build the project (run Qt uic code generator) to get "ui_qPlayerInitiativeView.h" resolved

#include "qplayerinitiativeview.h"
#include "ui_qplayerinitiativeview.h"


#include <qdebug.h>


qPlayerInitiativeView::qPlayerInitiativeView(QInitiativeTrackerWidget *parentTracker, QWidget *parent) :
        QWidget(parent), ui(new Ui::qPlayerInitiativeView) {
    ui->setupUi(this);

    loadEncounter(parentTracker->getEncounter());

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


