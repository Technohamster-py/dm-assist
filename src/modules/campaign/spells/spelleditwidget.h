#ifndef DM_ASSIST_SPELLEDITWIDGET_H
#define DM_ASSIST_SPELLEDITWIDGET_H

#include <QDialog>
#include <QJsonObject>
#include <QMap>


QT_BEGIN_NAMESPACE
namespace Ui { class SpellEditWidget; }
QT_END_NAMESPACE

class SpellEditWidget : public QDialog {
Q_OBJECT

public:
    explicit SpellEditWidget(QWidget *parent = nullptr);

    ~SpellEditWidget() override;

    QJsonObject toJson() const;
public:
    bool parseFromJson(QJsonObject json);
private:
    Ui::SpellEditWidget *ui;
    QJsonObject original;
    QString texture;

    QMap<QString, int> m_casting{
            {"action", 0},
            {"reaction", 1},
            {"bonus", 2},
            {"minute", 3},
            {"hour", 4}
    };

    QMap<QString, int> m_duration{
            {"round", 0},
            {"minute", 1},
            {"hour", 2},
            {"day", 3},
            {"year", 4},
            {"perm", 5}
    };

    QMap<QString, int> m_units{
            {"ft", 0},
            {"miles", 1}
    };

    QMap<QString, int> m_shapes{
            {"line", 0},
            {"cone", 1},
            {"sphere", 2},
            {"cube", 3},
            {"cylinder", 4},
            {"wall", 0}
    };
};


#endif //DM_ASSIST_SPELLEDITWIDGET_H
