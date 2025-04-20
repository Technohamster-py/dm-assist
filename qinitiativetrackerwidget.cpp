//
// Created by arseniy on 27.03.2024.
//
#include <QItemSelectionModel>

#include "qinitiativetrackerwidget.h"
#include "hpprogressbardelegate.h"
#include "ui_qinitiativetrackerwidget.h"


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
    QTableView *sharedWidget = new QTableView(sharedWindow);
    sharedWidget->setModel(model);
    auto *view = sharedWidget;

    // Делегат с начальным режимом отображения
    auto *hpDelegate = new HpProgressBarDelegate(HpProgressBarDelegate::Numeric, view);
    view->setItemDelegateForColumn(3, hpDelegate);

    connect(ui->hpModeBox, QOverload<int>::of(&QComboBox::currentIndexChanged), hpDelegate, [hpDelegate, view](int index) {
        hpDelegate->setDisplayMode(static_cast<HpProgressBarDelegate::DisplayMode>(index));
        view->viewport()->update();
    });

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
