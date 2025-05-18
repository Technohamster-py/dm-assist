#include <QItemSelectionModel>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>

#include "qinitiativetrackerwidget.h"
#include "hpprogressbardelegate.h"
#include "ui_qinitiativetrackerwidget.h"


#include <qdebug.h>



/**
 * @brief Constructs the QInitiativeTrackerWidget widget.
 * @param parent Pointer to the parent widget. Defaults to nullptr if no parent is provided.
 * @param sharedModel Pointer to the shared InitiativeModel instance. If nullptr, a new
 * InitiativeModel instance is created and assigned to the widget.
 * @details Initializes the widget's user interface and sets up necessary configurations.
 * The provided sharedModel is used if available; otherwise, a new InitiativeModel is created.
 * Calls setupUI() to configure additional UI elements.
 */
QInitiativeTrackerWidget::QInitiativeTrackerWidget(QWidget *parent, InitiativeModel *sharedModel)
        : QWidget(parent), ui(new Ui::QInitiativeTrackerWidget)
{
    model = sharedModel ? sharedModel : new InitiativeModel(this);
    ui->setupUi(this);

    setupUI();
}

/**
 * @brief Sets up the user interface elements and initializes connections.
 * @details Configures the table view with the associated data model and sets
 *          various properties like column resize mode and drop indicator visibility.
 *          Adds signal-slot connections to handle button clicks and table interactions.
 *          Specific functionalities include adding rows, navigation between turns,
 *          table sorting, and managing shared windows. Also, enables removal of a character
 *          when the appropriate cell is clicked in the table's designated column (column 5).
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
 * @brief Adds a new row to the initiative tracker table and sets focus to it.
 *
 * Creates a default `InitiativeCharacter` object with preset values for
 * name, initiative, armor class (AC), current hit points (HP), and maximum hit points (Max HP).
 * The newly created character is added to the underlying data model.
 *
 * After adding the new character, the focus is set to the newly created row in
 * the table view for ease of editing. Editing of the first cell in the new row
 * is initiated automatically.
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
 * @brief Opens a new shared window displaying the initiative tracker.
 *
 * This function creates a new secondary window to share the initiative tracker view.
 * The newly created window becomes a copy of the main initiative tracker view, and
 * it uses the same `InitiativeModel` instance. Any changes in the model are reflected
 * in the shared window in real time.
 *
 * - The shared window inherits the main table's model and hides specific columns, such as the delete column.
 * - A delegate for the "HP" column is applied to utilize a progress bar or numeric display mode.
 * - Updates to certain columns' visibility propagate to the shared windows.
 * - The shared window is added to an internal list (`sharedWindows`) to manage its lifetime and removal.
 *
 * Connections:
 * - Updates from the main window's HP display mode (`ui->hpModeBox`) alter the delegate's display mode in both main and shared windows.
 * - Signals for showing or hiding columns are relayed from the main widget to the shared widget.
 *
 * Window Properties:
 * - The window is configured to automatically delete upon closing (`Qt::WA_DeleteOnClose`), and its removal is tracked.
 * - The window has a preset size (800x400) and a custom title ("Shared Initiative Tracker").
 */
void QInitiativeTrackerWidget::openSharedWindow() {
    QWidget *sharedWindow = new QWidget;
    sharedWindow->setAttribute(Qt::WA_DeleteOnClose);
    sharedWindow->setWindowTitle("Shared Initiative Tracker");
    sharedWindow->resize(800, 400);

    sharedWindows.append(sharedWindow);
    connect(sharedWindow, &QObject::destroyed, this, [this, sharedWindow]()
            {
                sharedWindows.removeAll(sharedWindow);
            });

    QTableView *sharedWidget = new QTableView(sharedWindow);
    sharedWidget->setModel(model);
    auto *view = sharedWidget;
    sharedWidget->setColumnHidden(InitiativeModel::fields::del, true);

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
 * Advances the turn to the next entry in the initiative tracker.
 *
 * This function calculates the next entry in the tracker by incrementing the
 * current index and taking the modulus with the total number of rows to ensure
 * the index wraps around to the beginning when it reaches the end of the list.
 * It then updates the current index in the associated model.
 */
void QInitiativeTrackerWidget::nextTurn() {
    int next = (model->getCurrentIndex() + 1) % model->rowCount();
    model->setCurrentIndex(next);
}


/**
 * @brief Moves the current turn to the previous participant in the initiative tracker.
 *
 * This function is responsible for updating the current turn index to the previous
 * participant in the list maintained by the initiative model. If the current index
 * is the first in the list, the index wraps around to the last participant.
 *
 * @details
 * - If the model contains no rows, this function exits early to prevent division by zero.
 * - The turn is moved backward by decrementing the current index and cycling through
 *   the list using modulo arithmetic.
 */
void QInitiativeTrackerWidget::prevTurn() {
    int rowCount = model->rowCount();
    if (rowCount == 0) return; // Безопасность от деления на 0

    int current = model->getCurrentIndex();
    int previous = (current - 1 + rowCount) % rowCount;
    model->setCurrentIndex(previous);
}


/**
 * Sorts the initiative table within the widget.
 *
 * This method delegates the sorting task to the `InitiativeModel` instance
 * associated with the widget. It ensures that the entries in the table are
 * ordered based on their initiative values.
 */
void QInitiativeTrackerWidget::sortTable() {
    model->sortByInitiative();
}


/**
 * @brief Loads initiative data from a specified file into the model.
 *
 * This function delegates the task of loading data to the associated
 * `InitiativeModel` instance. The data in the specified file overwrites
 * the current data in the model.
 *
 * @param filename The path of the file to load the initiative data from.
 */
void QInitiativeTrackerWidget::loadFromFile(QString filename){
    model->loadFromFile(filename);
}


/**
 * @brief Saves the current initiative tracker data to a file.
 *
 * This function triggers the save operation in the associated InitiativeModel,
 * writing the current data to the specified file. The data is serialized in a
 * format determined by the InitiativeModel implementation.
 *
 * @param filename The name of the file to save the data to.
 */
void QInitiativeTrackerWidget::saveToFile(QString filename){
    model->saveToFile(filename);
}


/**
 * @brief Handles the click event for the save button, allowing the user to save the current encounter state to a file.
 *
 * This function opens a file dialog which enables the user to specify a location and name for saving the encounter
 * as an XML file. If a valid filename is selected, it then calls the saveToFile function to store the data.
 *
 * The default file path points to the user's documents directory, as determined by the system configuration.
 * The dialog filters for `.xml` files as the saving format.
 */
void QInitiativeTrackerWidget::on_saveButton_clicked(){
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save encounter to file"),
                                                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                    "Xml file (*.xml)");
    saveToFile(filename);
}

/**
 * @brief Handles the load button click event.
 *
 * This method is triggered when the user clicks the load button. It opens a file dialog
 * to allow the user to select an XML file representing an encounter. Once a file is selected,
 * its filename is passed to the loadFromFile method to load the encounter data into the initiative tracker.
 */
void QInitiativeTrackerWidget::on_loadButton_clicked(){
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Open encounter from file"),
                                                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                    "Xml file (*.xml)");
    loadFromFile(filename);
}


/**
 * Handles the close event for the QInitiativeTrackerWidget.
 *
 * This function is invoked when the widget is about to close. It prompts
 * the user to save the tracker configuration through a confirmation dialog.
 * If the user chooses to save, the `on_saveButton_clicked()` method is
 * invoked to save the tracker data. Additionally, it ensures that all
 * widgets in the `sharedWindows` list are closed before clearing the list.
 * Lastly, the base class's implementation of `closeEvent` is called.
 *
 * @param event A pointer to the QCloseEvent object containing details of the close event.
 */
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

/**
 * @brief Sets the visibility of a shared field in the initiative tracker.
 *
 * This function toggles the visibility of a specific column or shared field
 * in the initiative tracker widget based on the provided visibility parameter.
 * It emits appropriate signals to indicate the column visibility status
 * and the status of the field visibility change.
 *
 * @param index The index of the shared field or column to update.
 * @param visible A boolean indicating whether the field should be visible
 *                (true) or hidden (false).
 *
 * Emits:
 * - columnShown(int): If `visible` is true, the signal indicates that the
 *   column is now visible.
 * - columnHidden(int): If `visible` is false, the signal indicates that
 *   the column is now hidden.
 * - fieldVisibilityChanged(int, bool): Indicates that the visibility of
 *   the field at the given index has changed to the new visibility state.
 */
void QInitiativeTrackerWidget::setSharedFieldVisible(int index, bool visible) {
    if (visible){
        emit columnShown(index);
    } else{
        emit columnHidden(index);
    }
    emit fieldVisibilityChanged(index, visible);
}

/**
 * @brief Adds initiative tracking data from a specified file.
 *
 * This function delegates the task of loading data from the specified file
 * to the underlying model. The content of the file is expected to conform to
 * the format supported by the model's `addFromFile` functionality.
 *
 * @param filename The file path from which to load initiative tracking data.
 */
void QInitiativeTrackerWidget::addFromFile(QString filename) {
    model->addFromFile(filename);
}

/**
 * @brief Handles the click event for the "Add From File" button.
 *
 * This function opens a file dialog to allow the user to select an XML file
 * and then calls the addFromFile function with the selected file's path.
 * If no file is selected, no further action is taken.
 */
void QInitiativeTrackerWidget::on_addFromFileButton_clicked() {
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Save encounter to file"),
                                                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                    "Xml file (*.xml)");
    addFromFile(filename);
}

/**
 * @brief Sets the HP (hit points) display mode in the initiative tracker widget.
 *
 * This function adjusts the current index of the hpModeBox UI element
 * to reflect the specified mode for displaying hit points.
 *
 * @param mode The index representing the desired HP display mode.
 */
void QInitiativeTrackerWidget::setHpDisplayMode(int mode) {
    ui->hpModeBox->setCurrentIndex(mode);
}

/**
 * @brief Sets the visibility of the HP combo box within the initiative tracker widget.
 *
 * This function updates the visibility state of the HP mode combo box in the user interface.
 * The visibility is determined by the value of the `visible` parameter.
 *
 * @param visible An integer indicating whether the HP combo box should be visible (true/non-zero) or hidden (false/zero).
 */
void QInitiativeTrackerWidget::setHpComboBoxVisible(int visible) {
    ui->hpModeBox->setVisible(visible);
}
