#include "Platform.h"

Platform::Platform(qreal x, qreal y, qreal w, qreal h, QGraphicsItem* parent)
    : QGraphicsRectItem(parent)
{
    setRect(0, 0, w, h);
    setPos(x, y);
    setBrush(QBrush(QColor(90, 90, 90)));
}
