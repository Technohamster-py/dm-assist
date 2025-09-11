#include "rollwidget.h"
#include "ui_rollwidget.h"

#include <QRegularExpression>

RollWidget::RollWidget(QWidget *parent) :
        QWidget(parent), ui(new Ui::RollWidget) {
    ui->setupUi(this);

    connect(ui->commandButton, &QPushButton::clicked, [=](){ executeRoll(ui->rollEdit->text());});
    connect(ui->compactModeBox, &QCheckBox::toggled, [=](bool mode){m_compactMode = mode;});
    connectButtons();
}

RollWidget::~RollWidget() {
    delete ui;
}

/**
 * Executes a dice roll or modifier computation based on the given command string.
 *
 * This function processes a string representing a dice roll expression or arithmetic modifiers.
 * The command is parsed for tokens matching dice or modifier patterns. Results of the computation
 * are displayed in the user interface and stored in the internal last roll representation.
 *
 * The command can be in concise mode or standard format. Dice rolls are randomized, and any modifiers
 * or arithmetic operations provided in the command are applied to compute the final result.
 *
 * @param command A QString representing the dice roll command or modifiers. Examples of tokens include
 *        "2d6", "+5", "d20", or complex expressions like "2d6 + d4 - 3".
 *        Any whitespace in the original command is ignored during processing.
 *
 * @return The total value resulting from the dice rolls and modifiers parsed from the command string.
 *         This integer represents the final computed result of the expression.
 */
int RollWidget::executeRoll(QString command) {
    command = command.replace(" ", "");

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
        auto modMatch = modifierPattern.match(token);

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

    QString result = rollParts.join(" ");
    if (result.startsWith("+"))
        result.removeAt(0);
    m_lastRoll = QString::number(total) + " = " + result;

    ui->resultView->addItem(m_lastRoll);
    ui->resultView->scrollToBottom();

    return total;
}

/**
 * Creates a compact representation of a dice roll expression by grouping similar dice terms
 * and preserving arithmetic modifiers.
 *
 * This function processes an original dice roll expression, identifying and grouping dice rolls
 * with the same type while combining their counts. The numeric modifiers (e.g., "+5", "-3") are
 * preserved and included in the final compact form. The function ensures a minimal and unambiguous
 * representation of the input expression.
 *
 * @param original A QString containing the original dice roll expression or arithmetic modifiers.
 *        Expressions can include dice terms such as "2d6", "-d8", or numeric modifiers like "+5" or "-3".
 *
 * @return A QString representing the compacted dice roll expression. Dice terms are grouped, and modifiers
 *         are appended at the end. The resulting expression is simplified and formatted without unnecessary spaces.
 */
QString RollWidget::compactExpression(QString original) {
    auto it = tokenPattern.globalMatch(original);
    QMap<QString, QMap<int, int>> diceGroups;
    QStringList modifiers;

    while (it.hasNext()) {
        QString token = it.next().captured(0).trimmed();
        auto diceMatch = dicePattern.match(token);
        auto modMatch = modifierPattern.match(token);

        if (diceMatch.hasMatch()) {
            QString signStr = diceMatch.captured(1);
            int count = diceMatch.captured(2).isEmpty() ? 1 : diceMatch.captured(2).toInt();
            int sign = (signStr == "-") ? -1 : 1;
            QString die = "d" + diceMatch.captured(3);
            diceGroups[die][sign] += count;
        } else if (modMatch.hasMatch()){
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
    return result.join(" + ").replace(QRegularExpression(R"(\+\s*-)"), "- ").replace(" ", "");
}

void RollWidget::setCompactMode(bool mode) {
    ui->compactModeBox->setChecked(mode);
}

/**
 * Adds a new die or updates an existing die in the roll expression editor.
 *
 * This function modifies the current text in the roll expression editor based on the provided die code.
 * If the specified die is already included in the expression, the quantity of that die is incremented
 * or decremented depending on the `rightClick` parameter. If the die is not present, it is added as a new term.
 * The expression is updated dynamically, preserving the formatting, and the text field gains focus.
 *
 * @param dieCode A QString representing the die type to be added or modified (e.g., "d6", "d20").
 * @param rightClick A boolean indicating the user interaction mode. If true, the die count is decremented
 *        (or removed if the count becomes zero). If false, the die count is incremented.
 */
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

void RollWidget::updateTranslator() {
    ui->retranslateUi(this);
}
