#ifndef DM_ASSIST_SPELLEDITWIDGET_H
#define DM_ASSIST_SPELLEDITWIDGET_H

#include <QWidget>
#include <QFile>


QT_BEGIN_NAMESPACE
namespace Ui { class SpellEditWidget; }
QT_END_NAMESPACE

class SpellEditWidget : public QWidget {
Q_OBJECT

public:
    explicit SpellEditWidget(QWidget *parent = nullptr);
    SpellEditWidget(const QString& path, QWidget *parent = nullptr);

    ~SpellEditWidget() override;

protected:
    bool parseFromJson(const QString& jsonFile);
    bool saveToFile();
    void closeEvent(QCloseEvent *event);
private:
    Ui::SpellEditWidget *ui;

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

    QString m_file;
    QString m_textureFileName;
};


#endif //DM_ASSIST_SPELLEDITWIDGET_H
