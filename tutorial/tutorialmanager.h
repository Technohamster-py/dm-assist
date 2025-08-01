#ifndef DM_ASSIST_TUTORIALMANAGER_H
#define DM_ASSIST_TUTORIALMANAGER_H

#include <QVector>
#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPushButton>

struct TutorialStep {
    QString title;
    QString description;
    QWidget* target;
};


class TutorialOverlay : public QWidget{
Q_OBJECT
public:
    explicit TutorialOverlay(QWidget* parent = nullptr);

    void setTarget(QWidget* target);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QWidget* m_target = nullptr;
};


class TutorialManager : public QObject{
    Q_OBJECT
public:
    explicit TutorialManager(QWidget* parentWindow);

    void addStep(const TutorialStep& step);
    void start();
    void nextStep();
    void prevStep();
    void cancel();

private:
    void showCurrentStep();

    QVector<TutorialStep> m_steps;
    int m_currentIndex = -1;
    QWidget* m_parentWindow;
    TutorialOverlay* m_overlay;
    QLabel* m_titleLabel;
    QLabel* m_descriptionLabel;
    QPushButton* m_nextbutton;
    QPushButton* m_prevButton;
    QPushButton* closeButton;
    QWidget* m_instructionWidget;
};


#endif //DM_ASSIST_TUTORIALMANAGER_H
