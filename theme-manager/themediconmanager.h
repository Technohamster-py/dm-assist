#pragma once

#include <QObject>
#include <QPointer>
#include <QSize>
#include <QIcon>
#include <QList>
#include <QPointer>
#include <functional>

/**
 * @class ThemedIconManager
 * @brief A manager class responsible for handling themed icons within an application.
 *
 * The ThemedIconManager class manages and provides access to icons that are designed for
 * different themes, ensuring a consistent and dynamic visual representation within the
 * application. This class allows loading, retrieving, and switching icons based on active themes.
 *
 * It supports functionalities like loading theme-specific resources, fetching icons by theme,
 * and monitoring theme changes to dynamically update the displayed icons.
 *
 * This class is useful for applications with customizable themes or dynamic visual styling.
 */
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

static QPixmap renderIconInline(const QStringList& svgPaths, QSize iconSize = QSize(16, 16), int spacing = 2);

static QPixmap renderIconGrid(const QStringList& svgPaths, QSize iconSize = QSize(16, 16), int spacing = 2, int maxIconsPerRow = 3);

signals:
    void themeChanged();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    explicit ThemedIconManager(QObject* parent = nullptr);

    struct IconTarget {
        QString path;
        QSize size;
        QPointer<QObject> receiver;
        std::function<void(const QIcon&)> applyIcon;
        std::function<void(const QPixmap&)> applyPixmap;
    };

    QList<IconTarget> m_targets;

    static void regenerateAndApplyIcon(const IconTarget& target) ;
    void updateAllIcons();

static QColor themeColor();
};
