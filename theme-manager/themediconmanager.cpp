#include "themediconmanager.h"
#include <QApplication>
#include <QFile>
#include <QSvgRenderer>
#include <QPainter>
#include <QRegularExpression>
#include <QEvent>
#include <QDebug>
#include <QPalette>

ThemedIconManager::ThemedIconManager(QObject* parent)
        : QObject(parent)
{
    qApp->installEventFilter(this);
}

bool ThemedIconManager::eventFilter(QObject*, QEvent* event) {
    if (event->type() == QEvent::ApplicationPaletteChange)
        updateAllIcons();
    return false;
}

void ThemedIconManager::updateAllIcons() {
    m_targets.erase(std::remove_if(m_targets.begin(), m_targets.end(),
                                   [](const IconTarget& target) {
                                       return target.receiver.isNull();
                                   }),
                    m_targets.end());

    for (const auto& target : m_targets) {
        regenerateAndApplyIcon(target);
    }



    m_rawTargets.erase(std::remove_if(m_rawTargets.begin(), m_rawTargets.end(),
                                    [](const RawIconTarget& t) {
                                        return t.icon.isNull();
                                    }),
                       m_rawTargets.end());

    for (auto& t : m_rawTargets) {
        auto iconPtr = t.icon.toStrongRef();
        if (!iconPtr)
            continue;

        QFile file(t.path);
        if (!file.open(QIODevice::ReadOnly))
            continue;

        QString svg = QString::fromUtf8(file.readAll());
        file.close();

        svg.replace(QRegularExpression(R"(currentColor)"), themeColor().name());


        QSvgRenderer renderer(svg.toUtf8());
        QPixmap pixmap(t.size);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        renderer.render(&painter);
        painter.end();

        *iconPtr = QIcon(pixmap);
    }

}


void ThemedIconManager::regenerateAndApplyIcon(const IconTarget& target) {
    if (!target.receiver)
        return;

    QFile file(target.path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open SVG:" << target.path;
        return;
    }

    QString svg = QString::fromUtf8(file.readAll());
    file.close();

    svg.replace(QRegularExpression(R"(currentColor)"), themeColor().name());

    QSvgRenderer renderer(svg.toUtf8());
    QPixmap pixmap(target.size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    renderer.render(&painter);
    painter.end();

    QIcon icon(pixmap);

    if (target.applyIcon)
        target.applyIcon(icon);

    if (target.applyPixmap)
        target.applyPixmap(pixmap);
}



QColor ThemedIconManager::themeColor() const {
    return qApp->palette().color(QPalette::WindowText);
}

ThemedIconManager &ThemedIconManager::instance() {
    static ThemedIconManager inst;
    return inst;
}

void ThemedIconManager::addPixmapTarget(const QString &svgPath, QObject *receiver,
                                        std::function<void(const QPixmap &)> applyPixmap, QSize size) {

    if (!receiver || svgPath.isEmpty()) return;

    // Удаляем старую запись, если такая есть
    m_targets.erase(std::remove_if(m_targets.begin(), m_targets.end(),
                                   [receiver](const IconTarget& t) {
                                       return t.receiver == receiver;
                                   }),
                    m_targets.end());

    IconTarget target;
    target.path = svgPath;
    target.size = size;
    target.receiver = receiver;
    target.applyPixmap = applyPixmap;

    m_targets.append(target);
    regenerateAndApplyIcon(m_targets.last());
}

QIcon ThemedIconManager::addRawTarget(const QString &svgPath, QSize size) {
    auto iconPtr = QSharedPointer<QIcon>::create();
    RawIconTarget target;
    target.path = svgPath;
    target.size = size;
    target.icon = iconPtr;

    m_rawTargets.append(target);

    // Сгенерируем сразу
    QFile file(svgPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open SVG:" << svgPath;
        return *iconPtr;
    }

    QString svg = QString::fromUtf8(file.readAll());
    file.close();

    svg.replace(QRegularExpression(R"(currentColor)"), themeColor().name());


    QSvgRenderer renderer(svg.toUtf8());
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    renderer.render(&painter);
    painter.end();

    *iconPtr = QIcon(pixmap);

    return *iconPtr;
}

