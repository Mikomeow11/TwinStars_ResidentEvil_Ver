#ifndef SPIKE_H
#define SPIKE_H

#include <QBrush>
#include <QGraphicsPolygonItem>
#include <QPen>

class Spike : public QGraphicsPolygonItem {
public:
    explicit Spike(qreal x, qreal y, qreal w, qreal h, QGraphicsItem* parent = nullptr);

    enum { Type = UserType + 3 };
    int type() const override { return Type; }
};

#endif // SPIKE_H
