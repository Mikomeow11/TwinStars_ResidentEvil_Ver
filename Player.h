#ifndef PLAYER_H
#define PLAYER_H

#include <QBrush>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QObject>
#include <QSet>

class Player : public QObject, public QGraphicsRectItem {
    Q_OBJECT
public:
    Player(int id, QColor color, QGraphicsItem *parent = nullptr);
    void updatePosition(const QSet<int>& keys);

private:
    int playerId;
    qreal speed;
    qreal velocityY;
    qreal gravity;
    qreal jumpSpeed;
    bool isOnGround;
    bool jumpPressedLastFrame;
    bool facingRight;
    QGraphicsPixmapItem* spriteItem;
};

#endif // PLAYER_H
