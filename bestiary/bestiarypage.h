#ifndef DM_ASSIST_BESTIARYPAGE_H
#define DM_ASSIST_BESTIARYPAGE_H

#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class BestiaryPage; }
QT_END_NAMESPACE

class BestiaryPage : public QWidget {
Q_OBJECT

public:
    explicit BestiaryPage(QWidget *parent = nullptr);

    ~BestiaryPage() override;

private:
    Ui::BestiaryPage *ui;
};


#endif //DM_ASSIST_BESTIARYPAGE_H
