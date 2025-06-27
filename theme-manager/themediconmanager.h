#pragma once

#include <QObject>
#include <QSize>
#include <QIcon>
#include <QList>
#include <QPointer>
#include <functional>

class ThemedIconManager : public QObject {
Q_OBJECT
public:
    static ThemedIconManager& instance();

    template <typename T>
    void addIconTarget(const QString& svgPath, T* object, void (T::*setIconMethod)(const QIcon&), QSize size = QSize(24, 24)) {
        if (!object || svgPath.isEmpty()) return;

        IconTarget target;
        target.path = svgPath;
        target.size = size;
        target.receiver = object;
        target.apply = [object, setIconMethod](const QIcon& icon) {
            if (object)
                (object->*setIconMethod)(icon);
        };

        m_targets.append(target);
        regenerateAndApplyIcon(m_targets.last());
    }

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    ThemedIconManager(QObject* parent = nullptr);

    struct IconTarget {
        QString path;
        QSize size;
        QPointer<QObject> receiver;
        std::function<void(const QIcon&)> apply;
    };

    QList<IconTarget> m_targets;

    void regenerateAndApplyIcon(const IconTarget& target);
    void updateAllIcons();
    QColor themeColor() const;
};
