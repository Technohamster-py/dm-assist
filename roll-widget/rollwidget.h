#ifndef DM_ASSIST_ROLLWIDGET_H
#define DM_ASSIST_ROLLWIDGET_H

#include <QWidget>
#include <QRandomGenerator>
#include <QRegularExpression>

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

    QString compactExpression(QString original);
    bool compactMode() const {return m_compactMode;};

public slots:
    int executeRoll(QString command);
    void addDieToExpression(const QString& dieCode, bool rightClick);
    void setCompactMode(bool mode);
    void updateTranslator();

protected:
    QString m_lastRoll = "";

    QRegularExpression tokenPattern = QRegularExpression(R"(([+\-]?[\d]*[d|к]\d+|[+\-]?\d+))", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression dicePattern = QRegularExpression(R"(([+\-]?)\s*(\d*)[d|к](\d+))", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression modifierPattern = QRegularExpression(R"([+\-]?\s*\d+)", QRegularExpression::CaseInsensitiveOption);
private:
    Ui::RollWidget *ui;
    bool m_compactMode = false;

    void connectButtons();
};


#endif //DM_ASSIST_ROLLWIDGET_H
