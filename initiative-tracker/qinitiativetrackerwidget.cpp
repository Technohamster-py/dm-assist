#include <QItemSelectionModel>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>

#include "qinitiativetrackerwidget.h"
#include "hpprogressbardelegate.h"
#include "ui_qinitiativetrackerwidget.h"


#include <qdebug.h>


/**
 * @brief Конструктор: инициализация интерфейса.
 * @details Создает виджет трекера инициативы и инициализирует таблицу,
 * а также устанавливает связи сигналов и слотов.
 * Если передана модель sharedModel, она используется, иначе создается новая.
 */
QInitiativeTrackerWidget::QInitiativeTrackerWidget(QWidget *parent, InitiativeModel *sharedModel)
        : QWidget(parent), ui(new Ui::QInitiativeTrackerWidget)
{
    model = sharedModel ? sharedModel : new InitiativeModel(this);
    ui->setupUi(this);

    setupUI();
}

/**
 * @brief Настраивает элементы пользовательского интерфейса.
 * @details Устанавливает модель таблицы, режимы отображения и подключает кнопки
 * управления (добавление строки, переходы, сортировка, отображение общей таблицы).
 */
void QInitiativeTrackerWidget::setupUI() {
    ui->table->setModel(model);
    ui->table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->table->setDropIndicatorShown(true);

    connect(ui->addRowButton, &QPushButton::clicked, this, &QInitiativeTrackerWidget::addRow);
    connect(ui->nextButton, &QPushButton::clicked, this, &QInitiativeTrackerWidget::nextTurn);
    connect(ui->backButton, &QPushButton::clicked, this, &QInitiativeTrackerWidget::prevTurn);
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
 * @details Создает нового пустого персонажа с начальными значениями
 * и добавляет его в модель. После этого сразу переводит фокус
 * на созданную строку и активирует режим редактирования имени.
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

/**
 * @brief Открывает дополнительное окно с общей таблицей.
 * @details Создает новое окно с той же моделью персонажей. В таблицу устанавливается
 * делегат для отображения HP. Вариант отображения (числовой/графический) выбирается
 * в зависимости от текущего значения комбобокса.
 */
void QInitiativeTrackerWidget::openSharedWindow() {
    // Создаем новое окно
    QWidget *sharedWindow = new QWidget;
    sharedWindow->setAttribute(Qt::WA_DeleteOnClose);
    sharedWindow->setWindowTitle("Shared Initiative Tracker");
    sharedWindow->resize(800, 400);

    sharedWindows.append(sharedWindow);
    connect(sharedWindow, &QObject::destroyed, this, [this, sharedWindow]()
            {
                sharedWindows.removeAll(sharedWindow);
            });

    // Используем тот же экземпляр модели
    QTableView *sharedWidget = new QTableView(sharedWindow);
    sharedWidget->setModel(model);
    auto *view = sharedWidget;
    sharedWidget->setColumnHidden(InitiativeModel::fields::del, true);

    // Делегат с начальным режимом отображения
    auto *hpDelegate = new HpProgressBarDelegate(HpProgressBarDelegate::Numeric, view);
    view->setItemDelegateForColumn(3, hpDelegate);

    connect(ui->hpModeBox, QOverload<int>::of(&QComboBox::currentIndexChanged), hpDelegate, [hpDelegate, view](int index) {
        hpDelegate->setDisplayMode(static_cast<HpProgressBarDelegate::DisplayMode>(index));
        view->viewport()->update();
    });

    connect(this, SIGNAL(columnHidden(int)), sharedWidget, SLOT(hideColumn(int)));
    connect(this, SIGNAL(columnShown(int)), sharedWidget, SLOT(showColumn(int)));


    QVBoxLayout *layout = new QVBoxLayout(sharedWindow);
    layout->addWidget(sharedWidget);
    sharedWindow->setLayout(layout);
    sharedWindow->show();
}

/**
 * @brief Переходит к следующему ходу.
 * @details Увеличивает текущий индекс активного персонажа. При достижении
 * конца списка происходит переход к первому персонажу.
 */
void QInitiativeTrackerWidget::nextTurn() {
    int next = (model->getCurrentIndex() + 1) % model->rowCount();
    model->setCurrentIndex(next);
}

/**
 * @brief Переходит к предыдущему ходу.
 * @details Уменьшает текущий индекс активного персонажа.
 * Если индекс меньше нуля, происходит переход к последнему персонажу.
 */
void QInitiativeTrackerWidget::prevTurn() {
    int rowCount = model->rowCount();
    if (rowCount == 0) return; // Безопасность от деления на 0

    int current = model->getCurrentIndex();
    int previous = (current - 1 + rowCount) % rowCount;
    model->setCurrentIndex(previous);
}

/**
 * @brief Сортирует таблицу по инициативе.
 * @details Вызывает метод модели для сортировки списка персонажей
 * в порядке убывания инициативы.
 */
void QInitiativeTrackerWidget::sortTable() {
    model->sortByInitiative();
}

/**
 * @brief Загружает модель инициативы в файл
 * @param filename путь до файла
 */
void QInitiativeTrackerWidget::loadFromFile(QString filename){
    model->loadFromFile(filename);
}

/**
 * @brief Сохраняет модель инициативы в файл
 * @param filename путь до файла
 */
void QInitiativeTrackerWidget::saveToFile(QString filename){
    model->saveToFile(filename);
}


void QInitiativeTrackerWidget::on_saveButton_clicked(){
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save encounter to file"),
                                                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                    "Xml file (*.xml)");
    saveToFile(filename);
}

void QInitiativeTrackerWidget::on_loadButton_clicked(){
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Save encounter to file"),
                                                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                    "Xml file (*.xml)");
    loadFromFile(filename);
}


void QInitiativeTrackerWidget::closeEvent(QCloseEvent *event){
    QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                              tr("Сохранение конфигурации"),
                                                              tr("Сохранить трекер?"),
                                                              QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
        on_saveButton_clicked();

    for (QPointer<QWidget> w : sharedWindows) {
        if (w) w->close();
    }
    sharedWindows.clear();
    QWidget::closeEvent(event);
}

void QInitiativeTrackerWidget::setSharedFieldVisible(int index, bool visible) {
    if (visible){
        emit columnShown(index);
    } else{
        emit columnHidden(index);
    }
    emit fieldVisibilityChanged(index, visible);
}

void QInitiativeTrackerWidget::addFromFile(QString filename) {
    model->addFromFile(filename);
}

void QInitiativeTrackerWidget::on_addFromFileButton_clicked() {
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Save encounter to file"),
                                                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                    "Xml file (*.xml)");
    addFromFile(filename);
}

void QInitiativeTrackerWidget::setHpDisplayMode(int mode) {
    ui->hpModeBox->setCurrentIndex(mode);
}

void QInitiativeTrackerWidget::setHpComboBoxVisible(int visible) {
    ui->hpModeBox->setVisible(visible);
}
