#ifndef DM_ASSIST_ROLLWIDGET_H
#define DM_ASSIST_ROLLWIDGET_H

#include <QWidget>
#include <QRandomGenerator>


QT_BEGIN_NAMESPACE
namespace Ui { class RollWidget; }
QT_END_NAMESPACE

class RollWidget : public QWidget {
Q_OBJECT

public:
    explicit RollWidget(QWidget *parent = nullptr);

    ~RollWidget() override;

    static int rollDice(int diceValue){
        return QRandomGenerator::global()->bounded(1, diceValue+1);
    };

public slots:
    int executeRoll(QString command);

protected:
    QString m_lastRoll = "";

private:
    Ui::RollWidget *ui;
};


#endif //DM_ASSIST_ROLLWIDGET_H
