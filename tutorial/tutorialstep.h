#ifndef DM_ASSIST_TUTORIALSTEP_H
#define DM_ASSIST_TUTORIALSTEP_H

#include <QString>
#include <QWidget>

struct TutorialStep {
    QString title;
    QString description;
    QWidget* target;
};

#endif //DM_ASSIST_TUTORIALSTEP_H
