#include "rollwidget.h"
#include "ui_rollwidget.h"

#include <QRegularExpression>

RollWidget::RollWidget(QWidget *parent) :
        QWidget(parent), ui(new Ui::RollWidget) {
    ui->setupUi(this);

    connect(ui->commandButton, &QPushButton::clicked, [=](){ executeRoll(ui->rollEdit->text());});
    connect(ui->compactModeBox, &QCheckBox::toggled, this, &RollWidget::setCompactMode);
    connectButtons();
}

RollWidget::~RollWidget() {
    delete ui;
}

int RollWidget::executeRoll(QString command) {
    command = command.replace(" ", "");
    QRegularExpression tokenPattern(R"(([+\-]?[\d]*[d|к]\d+|[+\-]?\d+))", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression dicePattern(R"(([+\-]?)(\d*)[d|к](\d+))", QRegularExpression::CaseInsensitiveOption);
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
    ui->resultView->scrollToBottom();

    return total;
}

QString RollWidget::compactExpression(QString original) {
    QRegularExpression tokenPattern(R"([+\-]?\d*d\d+|[+\-]?\d+)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression dicePattern(R"(([+\-]?)(\d*)d(\d+))", QRegularExpression::CaseInsensitiveOption);

    auto it = tokenPattern.globalMatch(original);
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

void RollWidget::addDieToExpression(const QString &dieCode, bool rightClick) {
    QString text = ui->rollEdit->text().trimmed();
    if (text.isEmpty()) {
        ui->rollEdit->setText(dieCode);
        return;
    }

    QRegularExpression lastDiePattern(R"(([+\-]?)(\d*)(" + dieCode + R")\b)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression dicePattern(R"(([+\-]?)(\d*)d(\d+))");

    QRegularExpressionMatchIterator it = dicePattern.globalMatch(text);
    QString lastDie;
    int lastDieStart = -1;
    while (it.hasNext()) {
        auto match = it.next();
        lastDie = match.captured(0);
        lastDieStart = match.capturedStart(0);
    }

    if (!lastDie.isEmpty() && lastDie.endsWith(dieCode)) {
        QRegularExpression matchDie(R"(([+\-]?)(\d*)d(\d+))");
        auto m = matchDie.match(lastDie);
        if (m.hasMatch()) {
            int count = m.captured(2).isEmpty() ? 1 : m.captured(2).toInt();
            QString sign = m.captured(1).isEmpty() ? "+" : m.captured(1);
            QString newText;
            if (rightClick) {
                if (count <= 1) {
                    // Удалить
                    text.remove(lastDieStart, lastDie.length());
                } else {
                    newText = QString("%1%2%3").arg(sign).arg(count - 1).arg(dieCode);
                    text.replace(lastDieStart, lastDie.length(), newText);
                }
            } else {
                newText = QString("%1%2%3").arg(sign).arg(count + 1).arg(dieCode);
                text.replace(lastDieStart, lastDie.length(), newText);
            }
            ui->rollEdit->setText(text.trimmed());
            return;
        }
    }

    QString newDie = "+" + dieCode;
    ui->rollEdit->setText(text + " " + newDie);
    ui->rollEdit->setFocus();
}

void RollWidget::connectButtons() {
    connect(ui->d4Button, &QPushButton::clicked, [=](){
        bool rightClick = QGuiApplication::mouseButtons() & Qt::RightButton;
        addDieToExpression("d4", rightClick);
    });
    connect(ui->d6Button, &QPushButton::clicked, [=](){
        bool rightClick = QGuiApplication::mouseButtons() & Qt::RightButton;
        addDieToExpression("d6", rightClick);
    });
    connect(ui->d8Button, &QPushButton::clicked, [=](){
        bool rightClick = QGuiApplication::mouseButtons() & Qt::RightButton;
        addDieToExpression("d8", rightClick);
    });
    connect(ui->d10Button, &QPushButton::clicked, [=](){
        bool rightClick = QGuiApplication::mouseButtons() & Qt::RightButton;
        addDieToExpression("d10", rightClick);
    });
    connect(ui->d12Button, &QPushButton::clicked, [=](){
        bool rightClick = QGuiApplication::mouseButtons() & Qt::RightButton;
        addDieToExpression("d12", rightClick);
    });
    connect(ui->d20Button, &QPushButton::clicked, [=](){
        bool rightClick = QGuiApplication::mouseButtons() & Qt::RightButton;
        addDieToExpression("d20", rightClick);
    });
    connect(ui->d100Button, &QPushButton::clicked, [=](){
        bool rightClick = QGuiApplication::mouseButtons() & Qt::RightButton;
        addDieToExpression("d100", rightClick);
    });

}
