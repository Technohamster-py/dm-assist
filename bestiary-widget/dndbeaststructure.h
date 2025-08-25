#ifndef DM_ASSIST_DNDBEASTSTRUCTURE_H
#define DM_ASSIST_DNDBEASTSTRUCTURE_H

#include <QString>
#include <QMap>
#include <QObject>



struct BestiaryItem {
    QString name;
    QString description;
    QString type;
    int activationCost = 1;
};

#endif //DM_ASSIST_DNDBEASTSTRUCTURE_H
