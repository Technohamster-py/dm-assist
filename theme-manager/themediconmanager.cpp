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
    for (const IconTarget& target : m_targets)
        regenerateAndApplyIcon(target);
}

void ThemedIconManager::regenerateAndApplyIcon(const IconTarget& target) {
    QFile file(target.path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open SVG:" << target.path;
        return;
    }

    QString svg = QString::fromUtf8(file.readAll());
    file.close();

    svg.replace(QRegularExpression(R"(current-color)"), themeColor().name());

    QSvgRenderer renderer(svg.toUtf8());
    QPixmap pixmap(target.size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    renderer.render(&painter);
    painter.end();

    QIcon icon(pixmap);
    if (target.apply) {
        target.apply(icon);
    }
}

QColor ThemedIconManager::themeColor() const {
    return qApp->palette().color(QPalette::WindowText);
}

ThemedIconManager &ThemedIconManager::instance() {
    static ThemedIconManager inst;
    return inst;
}
