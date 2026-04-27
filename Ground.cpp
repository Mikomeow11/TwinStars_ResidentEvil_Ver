#include "Ground.h"

Ground::Ground(qreal x, qreal y, qreal w, qreal h, QGraphicsItem* parent)
    : QGraphicsRectItem(parent), hazardDamage(0)
{
    setRect(0, 0, w, h);
    setPos(x, y);
    setBrush(QBrush(QColor(60, 60, 60)));
}
