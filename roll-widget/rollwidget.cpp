#include "rollwidget.h"
#include "ui_rollwidget.h"

#include <QRegularExpression>

RollWidget::RollWidget(QWidget *parent) :
        QWidget(parent), ui(new Ui::RollWidget) {
    ui->setupUi(this);

    connect(ui->commandButton, &QPushButton::clicked, [=](){ executeRoll(ui->rollEdit->text());});

    connect(ui->compactModeBox, &QCheckBox::toggled, this, &RollWidget::setCompactMode);
}

RollWidget::~RollWidget() {
    delete ui;
}

int RollWidget::executeRoll(QString command) {
    command = command.replace(" ", "");
    QRegularExpression tokenPattern(R"(([+\-]?[\d]*d\d+|[+\-]?\d+))", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression dicePattern(R"(([+\-]?)(\d*)d(\d+))", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression modificatorPattern(R"([+\-]?\s?\d+)", QRegularExpression::CaseInsensitiveOption);

    if (compactMode())
        command = compactExpression(command);

    int total = 0;
    m_lastRoll.clear();
    QStringList rollParts;

    auto it = tokenPattern.globalMatch(command);
    while (it.hasNext()) {
        auto match = it.next();
        QString token = match.captured(1).trimmed();

        auto diceMatch = dicePattern.match(token);
        auto modMatch = modificatorPattern.match(token);

        if (diceMatch.hasMatch()) {
            QString signStr = diceMatch.captured(1);
            int count = diceMatch.captured(2).isEmpty() ? 1 : diceMatch.captured(2).toInt();
            int sides = diceMatch.captured(3).toInt();
            int sign = (signStr == "-") ? -1 : 1;

            QStringList rolls;
            int subtotal = 0;
            for (int i = 0; i < count; ++i) {
                int roll = QRandomGenerator::global()->bounded(1, sides + 1);
                subtotal += roll;
                rolls << QString::number(roll);
            }

            total += sign * subtotal;
            QString desc = QString("%1%2d%3[%4]")
                    .arg(sign < 0 ? "-" : "+")
                    .arg(count)
                    .arg(sides)
                    .arg(rolls.join(","));
            rollParts << desc;
        } else if (modMatch.hasMatch()){
            token = token.replace(" ", "");
            int mod = token.toInt();
            total += mod;
            rollParts << (mod >= 0 ? "+" : "") + QString::number(mod);
        } else continue;
    }

    m_lastRoll = rollParts.join(" ") + " = " + QString::number(total);
    if (m_lastRoll.startsWith("+"))
        m_lastRoll.removeAt(0);

    ui->resultView->addItem(m_lastRoll);

    return total;
}

void RollWidget::addDice(QString dice) {

}

QString RollWidget::compactExpression(QString original) {
    QRegularExpression tokenPattern(R"([+\-]?\d*d\d+|[+\-]?\d+)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression dicePattern(R"(([+\-]?)(\d*)d(\d+))", QRegularExpression::CaseInsensitiveOption);

    auto it = tokenPattern.globalMatch(original);
    // Ключ: "dX", значение: список со знаком (+/-) и количеством
    QMap<QString, QMap<int, int>> diceGroups;
    QStringList modifiers;

    while (it.hasNext()) {
        QString token = it.next().captured(0).trimmed();
        auto match = dicePattern.match(token);

        if (match.hasMatch()) {
            QString signStr = match.captured(1);
            int count = match.captured(2).isEmpty() ? 1 : match.captured(2).toInt();
            int sign = (signStr == "-") ? -1 : 1;
            QString die = "d" + match.captured(3);

            diceGroups[die][sign] += count;
        } else {
            modifiers << token;
        }
    }

    QStringList result;

    for (auto dieIt = diceGroups.constBegin(); dieIt != diceGroups.constEnd(); ++dieIt) {
        const QString& die = dieIt.key();
        const QMap<int, int>& counts = dieIt.value();

        for (auto countIt = counts.constBegin(); countIt != counts.constEnd(); ++countIt) {
            int sign = countIt.key();
            int count = countIt.value();

            if (count == 0) continue;

            QString part;
            if (count == 1)
                part = (sign == -1 ? "-" : "") + die;
            else
                part = QString("%1%2").arg(sign * count).arg(die);

            result << part;
        }
    }

    result.append(modifiers);
    return result.join(" + ").replace(QRegularExpression(R"(\+\s*-)"), "- ");
}

void RollWidget::setCompactMode(bool mode) {
 m_compactMode = mode;
}