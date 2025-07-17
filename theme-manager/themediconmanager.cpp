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
    emit themeChanged();
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
                                        std::function<void(const QPixmap &)> applyPixmap,
                                        bool override, QSize size) {

    if (!receiver || svgPath.isEmpty()) return;

    if (override)
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

QPixmap ThemedIconManager::renderIconInline(const QStringList &svgPaths, QSize iconSize, int spacing) {
    const qreal dpr = qApp->devicePixelRatio();

    int iconWidthPx = iconSize.width() * dpr;
    int iconHeightPx = iconSize.height() * dpr;
    int totalWidth = iconWidthPx * svgPaths.size() + spacing * dpr * (svgPaths.size() - 1);
    int totalHeight = iconHeightPx;

    QPixmap result(totalWidth, totalHeight);
    result.setDevicePixelRatio(dpr);
    result.fill(Qt::transparent);

    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    int x = 0;
    for (const auto& path : svgPaths) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly))
            continue;

        QString svg = QString::fromUtf8(file.readAll());
        file.close();

        svg.replace(QRegularExpression(R"(currentColor)"), themeColor().name());

        QSvgRenderer renderer(svg.toUtf8());
        QPixmap iconPixmap(iconWidthPx, iconHeightPx);
        iconPixmap.fill(Qt::transparent);

        QPainter p(&iconPixmap);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        renderer.render(&p);
        p.end();

        painter.drawPixmap(x, 0, iconPixmap);
        x += iconWidthPx + spacing * dpr;
    }

    painter.end();
    return result;
}

QPixmap ThemedIconManager::renderIconGrid(const QStringList &svgPaths, QSize iconSize, int spacing, int maxIconsPerRow) {
    if (svgPaths.isEmpty()) return {};

    const qreal dpr = qApp->devicePixelRatio();
    const int iconW = iconSize.width() * dpr;
    const int iconH = iconSize.height() * dpr;

    int iconsCount = svgPaths.size();

    int iconsPerRow = std::min(maxIconsPerRow, iconsCount);
    int rows = (svgPaths.size() + iconsPerRow - 1) / iconsPerRow;

    int totalWidth = iconsPerRow * iconW + spacing * dpr * (iconsPerRow - 1);
    int totalHeight = rows * iconH + spacing * dpr * (rows - 1);

    QPixmap pixmap(totalWidth, totalHeight);
    pixmap.setDevicePixelRatio(dpr);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    int x = 0, y = 0;
    int count = 0;
    for (const auto& path : svgPaths) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) continue;

        QString svg = QString::fromUtf8(file.readAll());
        file.close();

        svg.replace(QRegularExpression(R"(currentColor)"), themeColor().name());

        QSvgRenderer renderer(svg.toUtf8());
        QPixmap icon(iconW, iconH);
        icon.fill(Qt::transparent);

        QPainter iconPainter(&icon);
        iconPainter.setRenderHint(QPainter::Antialiasing);
        renderer.render(&iconPainter);
        iconPainter.end();

        painter.drawPixmap(x, y, icon);

        x += iconW + spacing * dpr;
        count++;

        if (count % iconsPerRow == 0) {
            x = 0;
            y += iconH + spacing * dpr;
        }
    }

    painter.end();
    return pixmap;
}
