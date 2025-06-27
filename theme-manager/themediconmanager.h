#pragma once

#include <QObject>
#include <QAbstractButton>
#include <QAction>
#include <QList>
#include <QSize>

class ThemedIconManager : public QObject {
Q_OBJECT
public:
    static ThemedIconManager& instance();

    void addControlledButton(const QString& svgPath, QAbstractButton* button, QSize size = QSize(24, 24));
    void addControlledAction(const QString& svgPath, QAction* action, QSize size = QSize(24, 24));

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    ThemedIconManager(QObject* parent = nullptr);

    struct IconTarget {
        QString path;
        QAbstractButton* button;
        QAction* action;
        QSize size;
    };

    QList<IconTarget> m_targets;

    void regenerateAndApplyIcon(const IconTarget& target);
    void updateAllIcons();
    QColor themeColor() const;
};
