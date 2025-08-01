#include "tutorialmanager.h"

#include <QPainter>
#include <QPainterPath>

void TutorialOverlay::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setBrush(QColor(0, 0, 0, 160));
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());

    if (m_target){
        QRect targetRect = m_target->geometry();
        QPoint topLeft = m_target->mapTo(this, QPoint(0, 0));
        targetRect.moveTopLeft(topLeft);

        QPainterPath path;
        path.addRect(rect());
        path.addRoundedRect(targetRect, 10, 10);

        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        painter.drawPath(path);
    }
}