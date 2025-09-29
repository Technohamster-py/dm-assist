#include "themediconmanager.h"
#include <QApplication>
#include <QFile>
#include <QSvgRenderer>
#include <QPainter>
#include <QRegularExpression>
#include <QEvent>
#include <QDebug>
#include <QPalette>
#include <utility>

/**
 * Constructs a ThemedIconManager instance.
 * Installs an event filter to monitor application-wide events.
 *
 * @param parent A QObject pointer representing the parent object of this instance.
 */
ThemedIconManager::ThemedIconManager(QObject* parent)
        : QObject(parent)
{
    qApp->installEventFilter(this);
}

/**
 * Filters events and handles application-level theme or palette changes.
 * Specifically, reacts to QEvent::ApplicationPaletteChange to update all managed icons.
 *
 * @param obj A QObject pointer representing the object that sent the event.
 *        This parameter is not used in the current implementation.
 * @param event A QEvent pointer representing the event to be filtered.
 *        This is used to check the event type and respond accordingly.
 * @return Returns false to indicate the event was not fully handled
 *         and should be processed further by other filters or the default handler.
 */
bool ThemedIconManager::eventFilter(QObject*, QEvent* event) {
    if (event->type() == QEvent::ApplicationPaletteChange)
        updateAllIcons();
    return false;
}

/**
 * Updates all icons managed by the ThemedIconManager.
 *
 * Removes invalid icon targets (those with null receivers) from the internal list.
 * Iterates through the remaining targets, regenerates, and reapplies their icons.
 * Emits the `themeChanged` signal to notify about the update.
 */
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


/**
 * Regenerates an icon or pixmap from an SVG file and applies it to the target.
 * Replaces the "currentColor" placeholder in the SVG with the current theme color,
 * renders the modified SVG to a pixmap, and applies the resulting QIcon or QPixmap
 * to the provided target by invoking its associated application method.
 *
 * @param target An IconTarget structure containing the path to the SVG file,
 *               the size for the pixmap, and the methods to apply the QIcon or QPixmap.
 *               If the target receiver is null, the operation is aborted.
 */
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


/**
 * Retrieves the current theme color used for icon rendering.
 *
 * The function extracts the theme color from the application's palette,
 * specifically the color used for `QPalette::WindowText`, which typically
 * matches the current theme or style settings of the application.
 *
 * @return The QColor representing the current theme color.
 */
QColor ThemedIconManager::themeColor() {
    return qApp->palette().color(QPalette::WindowText);
}

/**
 * Accesses the singleton instance of ThemedIconManager.
 * Creates the instance on first call and returns it for subsequent calls.
 *
 * @return A reference to the singleton instance of ThemedIconManager.
 */
ThemedIconManager &ThemedIconManager::instance() {
    static ThemedIconManager inst;
    return inst;
}

/**
 * Adds a target for rendering a themed QPixmap from an SVG file.
 * This allows updating the target with a new QPixmap when the theme changes.
 *
 * @param svgPath The path to the SVG file representing the icon.
 * @param receiver A QObject pointer representing the target object that will receive the pixmap.
 * @param applyPixmap A function to apply the rendered QPixmap to the target object.
 * @param override If true, removes any existing target associated with the receiver before adding the new one.
 * @param size The size of the pixmap to render.
 */
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
    target.applyPixmap = std::move(applyPixmap);

    m_targets.append(target);
    regenerateAndApplyIcon(m_targets.last());
}

/**
 * Renders a composite QPixmap by combining multiple SVG icons horizontally with specified spacing.
 * Each SVG icon is scaled to the specified size, and their color is themed dynamically.
 *
 * @param svgPaths A list of file paths to the SVG icons to be rendered.
 * @param iconSize The desired size of each individual icon within the QPixmap.
 * @param spacing The spacing in pixels between each icon in the resulting QPixmap.
 * @return A QPixmap containing the rendered icons arranged horizontally.
 */
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

/**
 * Renders a grid of icons based on the provided SVG file paths.
 * The method composes the icons into a grid layout with specified dimensions, spacing,
 * and maximum number of icons per row, while substituting theme-dependent color values
 * in the SVG content.
 *
 * @param svgPaths A list of file paths to SVG files which will be rendered as icons.
 * @param iconSize The size of each icon in the grid.
 * @param spacing The spacing to be applied between icons in the grid.
 * @param maxIconsPerRow The maximum number of icons to be placed in a single row.
 * @return A QPixmap object representing the rendered grid of icons. If the provided
 *         list of SVG paths is empty, an empty QPixmap is returned.
 */
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
