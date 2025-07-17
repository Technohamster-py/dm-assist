#ifndef DM_ASSIST_DNDCHARSHEETDIALOGS_H
#define DM_ASSIST_DNDCHARSHEETDIALOGS_H

#include <QDialog>
#include <QMap>
#include "dndmodels.h"

static QMap<int, QString> indexToStat = {
        {0, "str"},
        {1, "dex"},
        {2, "con"},
        {3, "int"},
        {4, "wis"},
        {5, "cha"}
};

static QMap<QString, int> statToIndex = {
        {"str", 0},
        {"dex", 1},
        {"con", 2},
        {"int", 3},
        {"wis", 4},
        {"cha", 5}
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
    explicit AttackDialog(QWidget *parent = nullptr, const Attack& ref = {});

    ~AttackDialog() override;

    Attack getCreatedAttack();

private:
    Ui::AttackDialog *ui;
    QString currentId = "";
};

#endif //DM_ASSIST_DNDCHARSHEETDIALOGS_H
