#include "Spike.h"

#include <QPolygonF>

Spike::Spike(qreal x, qreal y, qreal w, qreal h, QGraphicsItem* parent)
    : QGraphicsPolygonItem(parent)
{
    QPolygonF shape;
    shape << QPointF(0, h) << QPointF(w * 0.5, 0) << QPointF(w, h);
    setPolygon(shape);
    setPos(x, y);
    setBrush(QBrush(QColor(220, 60, 60)));
    setPen(QPen(Qt::NoPen));
}
