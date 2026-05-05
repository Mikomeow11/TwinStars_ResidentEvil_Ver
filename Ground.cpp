#include "Ground.h"
#include <QPixmap>
#include <QPen>


Ground::Ground(qreal x, qreal y, qreal w, qreal h, QGraphicsItem* parent)
    : QGraphicsRectItem(parent), hazardDamage(0)
{
    setRect(0, 0, w, h);
    setPos(x, y);

    QPixmap groundTex(":/assets/tiles/ground_tile.png");
    setBrush(QBrush(groundTex));
    setPen(Qt::NoPen);
}
