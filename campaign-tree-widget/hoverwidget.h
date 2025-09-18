#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QEvent>

enum class NodeType {
    Character,
    Encounter,
    Map,
    Music,
    Beast,
    Unknown
};

class HoverWidget : public QWidget {
Q_OBJECT

public:
    explicit HoverWidget(const QString &text, NodeType type, QWidget *parent = nullptr);

signals:
    void action1Clicked();
    void action2Clicked();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QLabel *label;
    QPushButton *action1;
    QPushButton *action2;
    NodeType m_type;

    void setupButtons(NodeType type);
};