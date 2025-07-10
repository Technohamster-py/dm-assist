#ifndef DM_ASSIST_STATUSEDITDIALOG_H
#define DM_ASSIST_STATUSEDITDIALOG_H

#include <QDialog>
#include <QMap>
#include <QSpinBox>
#include "initiativestructures.h"

QT_BEGIN_NAMESPACE
namespace Ui { class StatusEditDialog; }
QT_END_NAMESPACE

class StatusEditDialog : public QDialog {
Q_OBJECT

public:
    explicit StatusEditDialog(const InitiativeCharacter& character, QWidget *parent = nullptr);
    ~StatusEditDialog() override;
    InitiativeCharacter getUpdatedCharacter() const;

protected:
    InitiativeCharacter m_character;
    QList<Status> m_statuses;

    QMap<QString, QSpinBox*> m_standardStatusesMap;

    virtual void populate();
    virtual void addRow(Status status);
    QString m_currentIconPath = "";

private slots:
    void on_addButton_clicked();
    void on_iconButton_clicked();

private:
    Ui::StatusEditDialog *ui;
};


#endif //DM_ASSIST_STATUSEDITDIALOG_H
