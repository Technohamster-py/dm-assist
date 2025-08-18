#include "tutorialmanager.h"

#include <QPainter>
#include <QPainterPath>
#include <QHBoxLayout>

TutorialOverlay::TutorialOverlay(QWidget *parent): QWidget(parent) {
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setAttribute(Qt::WA_NoSystemBackground);

    setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);
    setGeometry(parent->rect());
    installEventFilter(this);
}

void TutorialOverlay::setTarget(QWidget *target) {
    m_target = target;
    update();
}

void TutorialOverlay::setInstructionWidget(QWidget *instr) {
    m_instructionWidget = instr;
}

bool TutorialOverlay::eventFilter(QObject *obj, QEvent *event) {
    return true; // block all events;
}

void TutorialOverlay::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setBrush(QColor(0, 0, 0, 160));
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());

    if (m_target){
        QRect targetRect = m_target->geometry();
        QPoint topLeft = m_target->mapTo(this, QPoint(0, 0));
        targetRect.moveTopLeft(topLeft);

        QPainterPath path;
        path.addRect(rect());
        path.addRoundedRect(targetRect, 10, 10);

        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        painter.drawPath(path);
    }
}

TutorialManager::TutorialManager(QWidget *parentWindow): QObject(parentWindow), m_parentWindow(parentWindow) {
    m_overlay = new TutorialOverlay(parentWindow);
    m_overlay->hide();

    m_instructionWidget = new QWidget(m_overlay);
    m_instructionWidget->setStyleSheet("background: white;"
                                       "border-radius: 10px;"
                                       "padding: 10px;");
    m_instructionWidget->setFixedWidth(300);
    m_titleLabel = new QLabel;
    m_titleLabel->setStyleSheet("font-weight: bold;"
                                "font-size: 16px;");

    m_descriptionLabel = new QLabel;
    m_descriptionLabel->setWordWrap(true);

    m_nextButton = new QPushButton(tr("Next"));
    m_prevButton = new QPushButton(tr("Back"));
    m_closeButton = new QPushButton(tr("Close"));

    auto* btnLayout = new QHBoxLayout;
    btnLayout->addWidget(m_prevButton);
    btnLayout->addWidget(m_nextButton);
    btnLayout->addWidget(m_closeButton);

    auto* layout = new QVBoxLayout(m_instructionWidget);
    layout->addWidget(m_titleLabel);
    layout->addWidget(m_descriptionLabel);
    layout->addLayout(btnLayout);

    connect(m_nextButton, &QPushButton::clicked, this, &TutorialManager::nextStep);
    connect(m_prevButton, &QPushButton::clicked, this, &TutorialManager::prevStep);
    connect(m_closeButton, &QPushButton::clicked, this, &TutorialManager::cancel);

    m_instructionWidget->setGraphicsEffect(m_opacityEffect);
    m_fadeAnimation = new QPropertyAnimation(m_opacityEffect, "opacity");
    m_fadeAnimation->setDuration(400);
}

void TutorialManager::addStep(const TutorialStep &step) {
    m_steps.append(step);
}

void TutorialManager::start() {
    m_currentIndex = 0;
    m_overlay->show();
    showCurrentStep();
}

void TutorialManager::nextStep() {
    if (m_currentIndex + 1 >= m_steps.size()){
        cancel();
        return;
    }
    ++m_currentIndex;
    showCurrentStep();
}

void TutorialManager::prevStep() {
    if (m_currentIndex <= 0)
        return;
    --m_currentIndex;
    showCurrentStep();
}

void TutorialManager::cancel() {
    m_overlay->hide();
    m_instructionWidget->hide();
    m_currentIndex = -1;
}

void TutorialManager::showCurrentStep() {
    if (m_currentIndex < 0 || m_currentIndex >= m_steps.size())
        return;

    TutorialStep step = m_steps[m_currentIndex];
    m_titleLabel->setText(step.title);
    m_descriptionLabel->setText(step.description);
    m_overlay->setTarget(step.target);

    m_overlay->raise();
    m_instructionWidget->raise();

    QRect targetRect = step.target->geometry();
    QPoint global = step.target->mapTo(m_parentWindow, QPoint(0, 0));
    m_instructionWidget->move(global + QPoint(0, targetRect.height() + 10));

    animateInstruction();
}

void TutorialManager::animateInstruction() {
    m_fadeAnimation->stop();
    m_opacityEffect->setOpacity(0.0);
    m_instructionWidget->show();

    m_fadeAnimation->setStartValue(0.0);
    m_fadeAnimation->setEndValue(1.0);
    m_fadeAnimation->start();
}

void TutorialManager::addSteps(const QVector<TutorialStep>& steps) {
    m_steps.append(steps);
}
