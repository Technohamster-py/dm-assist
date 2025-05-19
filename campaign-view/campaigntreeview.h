#ifndef DM_ASSIST_CAMPAIGNTREEVIEW_H
#define DM_ASSIST_CAMPAIGNTREEVIEW_H

#include <QWidget>
#include <QTreeView>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QToolButton>
#include <QStyledItemDelegate>
#include <QApplication>
#include <QMouseEvent>
#include <QPainter>


class OpenButtonDelegate: public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit OpenButtonDelegate(QObject* parent = nullptr): QStyledItemDelegate(parent){}

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

signals:
    void openFileRequested(const QModelIndex& index) const;

private:
    QRect buttonRect(const QStyleOptionViewItem& option) const;
};

class CampaignTreeView : public QWidget {
    Q_OBJECT
public:
    explicit CampaignTreeView(const QString& rootPath, QWidget* parent = nullptr);

signals:
    void fileOpened(const QString& filePath);
    void editRequested(const QString& itemPath);

private slots:
    void onItemDoubleClicked(const QModelIndex& index);
    void onOpenButtonClicked(const QModelIndex& index);

private:
    QTreeView* treeView;
    QFileSystemModel* model;
};


#endif //DM_ASSIST_CAMPAIGNTREEVIEW_H
