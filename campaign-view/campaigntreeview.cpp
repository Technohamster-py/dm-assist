#include "campaigntreeview.h"

void OpenButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyledItemDelegate::paint(painter, option, index);

    auto fsModel = qobject_cast<QFileSystemModel*>(const_cast<QAbstractItemModel*>(index.model()));
    if (!fsModel || fsModel->fileInfo(index).isDir()) return;

    QStyleOptionButton button;
    button.rect = buttonRect(option);
    button.text = tr("Open");
    button.state = QStyle::State_Enabled;

    QApplication::style()->drawControl(QStyle::CE_PushButton, &button, painter);
}

QRect OpenButtonDelegate::buttonRect(const QStyleOptionViewItem &option) const {
    return QRect(option.rect.right() - 70, option.rect.top() + 4, 60, option.rect.height() - 8);
}

bool OpenButtonDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                                     const QModelIndex &index) {
    auto fsModel = qobject_cast<QFileSystemModel*>(const_cast<QAbstractItemModel*>(index.model()));
    if (!fsModel || fsModel->fileInfo(index).isDir()) return false;

    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (buttonRect(option).contains(mouseEvent->pos())) {
            emit openFileRequested(index);
            return true;
        }
    }
    return false;
}

CampaignTreeView::CampaignTreeView(const QString &rootPath, QWidget *parent) : QWidget(parent){
    treeView = new QTreeView(this);
    model = new QFileSystemModel(this);
    model->setRootPath(rootPath);
    model->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    model->setNameFilters(QStringList() << "*.xml");
    model->setNameFilterDisables(false);

    treeView->setModel(model);
    treeView->setRootIndex(model->index(rootPath));
    treeView->setColumnHidden(1, true);
    treeView->setColumnHidden(2, true);
    treeView->setColumnHidden(3, true);
    treeView->setHeaderHidden(true);

    auto* delegate = new OpenButtonDelegate(this);
    treeView->setItemDelegate(delegate);

    connect(treeView, &QTreeView::doubleClicked, this, &CampaignTreeView::onItemDoubleClicked);
    connect(delegate, &OpenButtonDelegate::openFileRequested, this, &CampaignTreeView::onOpenButtonClicked);

    auto* layout = new QHBoxLayout(this);
    layout->addWidget(treeView);
    setLayout(layout);
}

void CampaignTreeView::onItemDoubleClicked(const QModelIndex &index) {
    if (model->isDir(index)) return;
    emit editRequested(model->filePath(index));
}

void CampaignTreeView::onOpenButtonClicked(const QModelIndex &index) {
    emit fileOpened(model->filePath(index));
}
