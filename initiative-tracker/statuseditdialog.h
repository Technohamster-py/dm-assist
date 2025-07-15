#ifndef DM_ASSIST_STATUSEDITDIALOG_H
#define DM_ASSIST_STATUSEDITDIALOG_H

#include <QDialog>
#include <QMap>
#include <QSpinBox>
#include "initiativestructures.h"
#include <QAbstractTableModel>

class StatusModel : public QAbstractTableModel {
Q_OBJECT
public:
    enum fields {
        icon,
        title,
        timer,
        del
    };
    explicit StatusModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    Status statusAt(int row);
    void addStatus(const Status &status);
    void remove(int row);

private:
    QVector<Status> statuses;
};


QT_BEGIN_NAMESPACE
namespace Ui { class StatusEditDialog; }
QT_END_NAMESPACE

class StatusEditDialog : public QDialog {
Q_OBJECT

public:
    explicit StatusEditDialog(QList<Status> statuses, QWidget *parent = nullptr);
    ~StatusEditDialog() override;

    QList<Status> statuses() const;

protected:
    InitiativeCharacter m_character;
    QList<Status> m_statuses;

    QMap<QString, QSpinBox*> m_standardStatusesMap;

    virtual void populate();
    QString m_currentIconPath = ":/statuses/status-blinded.svg";

private slots:
    void on_addButton_clicked();
    void on_iconButton_clicked();

private:
    Ui::StatusEditDialog *ui;

    StatusModel* model;

    void closeEvent(QCloseEvent *event);
};


#endif //DM_ASSIST_STATUSEDITDIALOG_H
