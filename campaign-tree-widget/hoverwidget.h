#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QEvent>

class HoverWidget : public QWidget {
Q_OBJECT

public:
    explicit HoverWidget(const QString &text, QWidget *parent = nullptr);

signals:
    void editClicked();
    void activateClicked();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QLabel *label;
    QPushButton *editButton;
    QPushButton *activateButton;
};