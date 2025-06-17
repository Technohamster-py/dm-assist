#ifndef DM_ASSIST_DNDCHARSHEETDIALOGS_H
#define DM_ASSIST_DNDCHARSHEETDIALOGS_H

#include <QDialog>
#include <QMap>
#include "dndmodels.h"

static QMap<int, QString> statsMap = {
        {0, "str"},
        {1, "dex"},
        {2, "con"},
        {3, "int"},
        {4, "wis"},
        {5, "cha"}
};

QT_BEGIN_NAMESPACE
namespace Ui { class ResourceDialog; class AttackDialog;}
QT_END_NAMESPACE

class ResourceDialog : public QDialog {
Q_OBJECT

public:
    explicit ResourceDialog(QWidget *parent = nullptr);
    ~ResourceDialog() override;

    Resource getCreatedResource();

private:
    Ui::ResourceDialog *ui;
};




class AttackDialog : public QDialog {
Q_OBJECT

public:
    explicit AttackDialog(QWidget *parent = nullptr);

    ~AttackDialog() override;

    Attack getCreatedAttack();

private:
    Ui::AttackDialog *ui;
};

#endif //DM_ASSIST_DNDCHARSHEETDIALOGS_H
