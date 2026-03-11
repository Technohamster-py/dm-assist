#include "iconpickerdialog.h"
#include <QVBoxLayout>
#include <QSvgRenderer>
#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>

#include "themediconmanager.h"

/**
 * @brief Constructs an IconLabel object with the specified SVG icon and parent widget.
 *
 * This constructor initializes an IconLabel by rendering the provided SVG icon,
 * applying it as a pixmap, and setting additional display properties such as
 * cursor style, alignment, and tooltip. It also registers the label as a pixmap
 * target with the ThemedIconManager for dynamic theme updates.
 *
 * @param iconPath The file path of the SVG icon to be displayed.
 * @param parent The parent widget for this label. Default is nullptr.
 * @return A constructed IconLabel object.
 */
IconLabel::IconLabel(const QString &iconPath, QWidget *parent)
        : QLabel(parent), m_iconPath(iconPath) {
    QPixmap pix(40, 40);
    pix.fill(Qt::transparent);
    QSvgRenderer renderer(iconPath);
    QPainter painter(&pix);
    renderer.render(&painter);

    ThemedIconManager::instance().addPixmapTarget(iconPath, this, [=](const QPixmap &px){ setPixmap(px);});
    setToolTip(iconPath);
    setFrameShape(QFrame::NoFrame);
    setAlignment(Qt::AlignCenter);
    setCursor(Qt::PointingHandCursor);

    installEventFilter(parent);
}

/**
 * @brief Retrieves the file path of the SVG icon associated with this label.
 *
 * This function returns the file path of the currently assigned SVG icon,
 * which is used for rendering the label's pixmap.
 *
 * @return A QString containing the file path of the SVG icon.
 */
QString IconLabel::iconPath() const {
    return m_iconPath;
}

/**
 * @brief Handles mouse press events for the IconLabel widget.
 *
 * This method overrides the default mousePressEvent of QLabel. It emits the
 * clicked signal when the left mouse button is pressed. The base implementation
 * of the event is also called to ensure proper event propagation.
 *
 * @param event The QMouseEvent containing information about the mouse press event.
 */
void IconLabel::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
    QLabel::mousePressEvent(event);
}

/**
 * @brief Handles the mouse double-click event for the IconLabel.
 *
 * This method is triggered when the user performs a double-click action on the label.
 * If the event is associated with the left mouse button, it emits the `doubleClicked()` signal.
 * It also ensures that the base class implementation of the event is called.
 *
 * @param event The QMouseEvent containing details about the double-click action.
 */
void IconLabel::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit doubleClicked();
    }
    QLabel::mouseDoubleClickEvent(event);
}


/**
 * @brief Opens an icon picker dialog and returns the path of the selected icon.
 *
 * This method creates an instance of the IconPickerDialog, executes it as a modal dialog,
 * and retrieves the file path of the icon selected by the user. The dialog must be closed
 * either by selecting an icon or canceling the operation.
 *
 * @param parent The parent widget for the dialog. Default is nullptr.
 * @return The file path of the selected icon, or an empty string if no icon was selected.
 */
QString IconPickerDialog::getSelectedIcon(QWidget *parent) {
    IconPickerDialog dlg(parent);
    dlg.exec();
    return dlg.selectedIconPath;
}

/**
 * @brief Constructs an IconPickerDialog object with a scrollable grid layout to display icons.
 *
 * This constructor initializes the IconPickerDialog, setting up its minimum size, drag-and-drop
 * properties, and creating a scrollable container where icons will be displayed in a grid layout.
 * It sets the main layout of the dialog and invokes the loadIcons() method to populate the grid
 * with icons.
 *
 * @param parent The parent widget for this dialog. Default is nullptr.
 */
IconPickerDialog::IconPickerDialog(QWidget *parent)
        : QDialog(parent), scrollArea(new QScrollArea(this)), gridLayout(new QGridLayout()) {
    setAcceptDrops(true);
    setMinimumSize(400, 300);

    auto *container = new QWidget();
    container->setLayout(gridLayout);
    scrollArea->setWidget(container);
    scrollArea->setWidgetResizable(true);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(scrollArea);
    setLayout(mainLayout);

    loadIcons();
}

/**
 * @brief Loads SVG icons from a predefined directory and organizes them in a grid layout.
 *
 * This method scans the specified icon directory for SVG files, ensuring the directory exists
 * by creating it if necessary. It iterates through the discovered SVG files and adds each icon
 * to a grid layout within the dialog. Icons are arranged in rows and columns, with a maximum
 * number of columns determined by an internal constant.
 *
 * Each row is populated sequentially. When the maximum number of columns is reached, the method
 * starts a new row.
 *
 * @details The method uses `QDir` to handle directory interactions and retrieves files with
 * a `.svg` extension via a filter. For every file, the full path is computed and passed to
 * the helper method `addIconToGrid()` to display the icon in the grid layout.
 */
void IconPickerDialog::loadIcons() {
    QDir dir(iconDirectory);
    if (!dir.exists()) dir.mkpath(".");

    QStringList files = dir.entryList(QStringList() << "*.svg", QDir::Files);
    int row = 0, col = 0;
    const int maxCols = 4;

    for (const QString &file : files) {
        QString fullPath = dir.absoluteFilePath(file);
        addIconToGrid(fullPath);
        if (++col >= maxCols) {
            col = 0;
            ++row;
        }
    }
}

/**
 * @brief Adds an icon to the grid layout and connects its signals for interaction.
 *
 * This function creates an instance of IconLabel for the given SVG file path,
 * adds it to the next available position in a 4-column grid layout, and connects
 * its `clicked` and `doubleClicked` signals to perform specific actions. The
 * click event highlights the label, while the double-click event sets the selected
 * icon path, emits the `iconSelected` signal, and closes the dialog.
 *
 * @param filePath The file path of the SVG icon to be added to the grid.
 */
void IconPickerDialog::addIconToGrid(const QString &filePath) {

    auto *iconLabel = new IconLabel(filePath, this);

    int index = gridLayout->count();
    int row = index / 4;
    int col = index % 4;
    gridLayout->addWidget(iconLabel, row, col);

    connect(iconLabel, &IconLabel::clicked, [this, iconLabel]() {
            highlightLabel(iconLabel);
    });

    connect(iconLabel, &IconLabel::doubleClicked, [this, iconLabel]() {
            selectedIconPath = iconLabel->toolTip();
            emit iconSelected(selectedIconPath);
            accept();
    });
}

/**
 * @brief Highlights the specified IconLabel by applying a visual effect.
 *
 * This method sets a dashed border around the provided IconLabel to indicate
 * that it is highlighted. Any previously highlighted label will have its
 * highlight effect removed before applying the highlight to the new label.
 *
 * @param label The IconLabel to be highlighted.
 */
void IconPickerDialog::highlightLabel(IconLabel *label) {
    if (highlightedLabel) {
        highlightedLabel->setStyleSheet("");
    }
    label->setStyleSheet("border: 2px dashed palette(Highlight);");
    highlightedLabel = label;
}

/**
 * @brief Handles the loss of focus event for the IconPickerDialog.
 *
 * This method is triggered when the IconPickerDialog loses focus. It calls
 * the base class implementation of the focusOutEvent and then rejects the
 * dialog, effectively closing it without a selection.
 *
 * @param event The focus event containing details about the focus change.
 */
void IconPickerDialog::focusOutEvent(QFocusEvent *event) {
    QDialog::focusOutEvent(event);
    reject(); // Закрыть без выбора
}

/**
 * @brief Handles drag enter events to allow dragging SVG files into the dialog.
 *
 * This method is invoked when a drag operation enters the dialog's bounds. It checks
 * the drag event's MIME data for URLs and filters those ending with ".svg". If a valid
 * SVG file URL is detected, the proposed action for the drag operation is accepted.
 *
 * @param event The QDragEnterEvent containing information about the drag operation.
 */
void IconPickerDialog::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        for (const QUrl &url : urls) {
            if (url.toLocalFile().endsWith(".svg")) {
                event->acceptProposedAction();
                return;
            }
        }
    }
}

/**
 * @brief Handles the drop event to add SVG icons to the icon grid.
 *
 * This method processes a drag-and-drop operation by extracting the list of files
 * dropped into the widget. It filters for SVG files, copies them to the designated
 * icon directory, and then displays them in the icon grid of the dialog.
 *
 * @param event The QDropEvent containing the drop action data, including MIME data
 * that holds the list of files being dropped.
 */
void IconPickerDialog::dropEvent(QDropEvent *event) {
    QList<QUrl> urls = event->mimeData()->urls();
    QDir dir(iconDirectory);
    for (const QUrl &url : urls) {
        QString local = url.toLocalFile();
        if (local.endsWith(".svg")) {
            QString dest = dir.filePath(QFileInfo(local).fileName());
            QFile::copy(local, dest);
            addIconToGrid(dest);
        }
    }
}
