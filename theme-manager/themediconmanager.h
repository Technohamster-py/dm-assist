#pragma once

#include <QObject>
#include <QPointer>
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
        if (!object || svgPath.isEmpty())
            return;

        m_targets.erase(std::remove_if(m_targets.begin(), m_targets.end(),
                                       [object](const IconTarget& target) {
                                           return target.receiver == object;
                                       }),
                        m_targets.end());

        IconTarget target;
        target.path = svgPath;
        target.size = size;
        target.receiver = object;
        target.applyIcon = [object, setIconMethod](const QIcon& icon) {
            if (object)
                (object->*setIconMethod)(icon);
        };

        m_targets.append(target);
        regenerateAndApplyIcon(m_targets.last());
    }

    void addPixmapTarget(const QString &svgPath, QObject *receiver, std::function<void(const QPixmap &)> applyPixmap,
                         bool override = true, QSize size = QSize(24, 24));


protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    ThemedIconManager(QObject* parent = nullptr);

    struct IconTarget {
        QString path;
        QSize size;
        QPointer<QObject> receiver;
        std::function<void(const QIcon&)> applyIcon;
        std::function<void(const QPixmap&)> applyPixmap;
    };

    QList<IconTarget> m_targets;

    void regenerateAndApplyIcon(const IconTarget& target);
    void updateAllIcons();
    QColor themeColor() const;
};
