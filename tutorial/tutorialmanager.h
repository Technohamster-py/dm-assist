#ifndef DM_ASSIST_TUTORIALMANAGER_H
#define DM_ASSIST_TUTORIALMANAGER_H

#include <QVector>
#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

#include "tutorialstep.h"


class TutorialOverlay : public QWidget{
Q_OBJECT
public:
    explicit TutorialOverlay(QWidget* parent = nullptr);

    void setTarget(QWidget* target);
    void setInstructionWidget(QWidget* instr);

protected:
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    QWidget* m_target = nullptr;
    QWidget* m_instructionWidget = nullptr;
};


class TutorialManager : public QObject{
    Q_OBJECT
public:
    explicit TutorialManager(QWidget* parentWindow);

public slots:
    void addStep(const TutorialStep& step);
    void addSteps(const QVector<TutorialStep>& steps);
    void start();
    void nextStep();
    void prevStep();
    void cancel();

private:
    void showCurrentStep();
    void animateInstruction();

    QVector<TutorialStep> m_steps;
    int m_currentIndex = -1;

    QWidget* m_parentWindow;
    TutorialOverlay* m_overlay;
    QLabel* m_titleLabel;
    QLabel* m_descriptionLabel;
    QPushButton* m_nextButton;
    QPushButton* m_prevButton;
    QPushButton* m_closeButton;
    QWidget* m_instructionWidget;

    QGraphicsOpacityEffect* m_opacityEffect;
    QPropertyAnimation* m_fadeAnimation;
};


#endif //DM_ASSIST_TUTORIALMANAGER_H
