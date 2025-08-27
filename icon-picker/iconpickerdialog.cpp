#include "iconpickerdialog.h"
#include <QVBoxLayout>
#include <QSvgRenderer>
#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>

#include "themediconmanager.h"

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

QString IconLabel::iconPath() const {
    return m_iconPath;
}

void IconLabel::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
    QLabel::mousePressEvent(event);
}

void IconLabel::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit doubleClicked();
    }
    QLabel::mouseDoubleClickEvent(event);
}




QString IconPickerDialog::getSelectedIcon(QWidget *parent) {
    IconPickerDialog dlg(parent);
    dlg.exec();
    return dlg.selectedIconPath;
}

IconPickerDialog::IconPickerDialog(QWidget *parent)
        : QDialog(parent), scrollArea(new QScrollArea(this)), gridLayout(new QGridLayout()) {
    setAcceptDrops(true);
    setMinimumSize(400, 300);

    QWidget *container = new QWidget();
    container->setLayout(gridLayout);
    scrollArea->setWidget(container);
    scrollArea->setWidgetResizable(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(scrollArea);
    setLayout(mainLayout);

    loadIcons();
}

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

void IconPickerDialog::addIconToGrid(const QString &filePath) {

    IconLabel *iconLabel = new IconLabel(filePath, this);

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

void IconPickerDialog::highlightLabel(IconLabel *label) {
    if (highlightedLabel) {
        highlightedLabel->setStyleSheet("");
    }
    label->setStyleSheet("border: 2px dashed palette(Highlight);");
    highlightedLabel = label;
}

void IconPickerDialog::focusOutEvent(QFocusEvent *event) {
    QDialog::focusOutEvent(event);
    reject(); // Закрыть без выбора
}

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
