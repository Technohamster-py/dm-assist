#include "themediconmanager.h"
#include <QApplication>
#include <QFile>
#include <QSvgRenderer>
#include <QPainter>
#include <QRegularExpression>
#include <QEvent>
#include <QDebug>

ThemedIconManager::ThemedIconManager(QObject* parent)
        : QObject(parent)
{
    qApp->installEventFilter(this);
}

ThemedIconManager& ThemedIconManager::instance() {
    static ThemedIconManager inst;
    return inst;
}

void ThemedIconManager::addControlledButton(const QString& svgPath, QAbstractButton* button, QSize size) {
    if (!button || svgPath.isEmpty())
        return;

    m_targets.append({ svgPath, button, nullptr, size });
    regenerateAndApplyIcon(m_targets.last());
}

void ThemedIconManager::addControlledAction(const QString& svgPath, QAction* action, QSize size) {
    if (!action || svgPath.isEmpty())
        return;

    m_targets.append({ svgPath, nullptr, action, size });
    regenerateAndApplyIcon(m_targets.last());
}

void ThemedIconManager::updateAllIcons() {
    for (const auto& target : m_targets) {
        regenerateAndApplyIcon(target);
    }
}

void ThemedIconManager::regenerateAndApplyIcon(const IconTarget& target) {
    QFile file(target.path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open SVG:" << target.path;
        return;
    }

    QString svg = QString::fromUtf8(file.readAll());
    file.close();

    svg.replace(QRegularExpression(R"(fill\s*=\s*["'][^"']*["'])"),
                QString(R"(fill="%1")").arg(themeColor().name()));

    QSvgRenderer renderer(svg.toUtf8());
    QPixmap pixmap(target.size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    renderer.render(&painter);
    painter.end();

    QIcon icon(pixmap);

    if (target.button)
        target.button->setIcon(icon);
    if (target.action)
        target.action->setIcon(icon);
}

QColor ThemedIconManager::themeColor() const {
    return qApp->palette().color(QPalette::WindowText);
}

bool ThemedIconManager::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::ApplicationPaletteChange) {
        updateAllIcons();
    }
    return QObject::eventFilter(obj, event);
}
