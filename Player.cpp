#include "Player.h"

#include "Ground.h"
#include "Platform.h"
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QGraphicsScene>
#include <Qt>

Player::Player(int id, QColor color, QGraphicsItem *parent)
    : QGraphicsRectItem(parent),
      playerId(id),
      speed(5.0),
      velocityY(0.0),
      gravity(0.8),
      jumpSpeed(14.0),
      isOnGround(false),
      jumpPressedLastFrame(false),
      facingRight(true),
      spriteItem(nullptr)
{
    setRect(0, 0, 64, 96);
    setBrush(Qt::NoBrush);
    setPen(Qt::NoPen);

    QString spritePath;
    if (playerId == 1) {
        spritePath = ":/assets/characters/p1_idle.png.png";
    } else {
        spritePath = ":/assets/characters/p2_idle.png.png";
    }

    QPixmap sprite(spritePath);
    if (!sprite.isNull()) {
        spriteItem = new QGraphicsPixmapItem(this);
        spriteItem->setPixmap(sprite.scaled(rect().width(), rect().height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        spriteItem->setPos(0, 0);
    } else {
        // Fallback: keep old colored rectangle when sprite loading fails.
        setBrush(QBrush(color));
    }
}

void Player::updatePosition(const QSet<int>& keys) {
    constexpr qreal kEpsilon = 0.1;
    qreal dx = 0.0;

    const int leftKey = (playerId == 1) ? Qt::Key_A : Qt::Key_Left;
    const int rightKey = (playerId == 1) ? Qt::Key_D : Qt::Key_Right;
    const int jumpKey = (playerId == 1) ? Qt::Key_W : Qt::Key_Up;

    if (keys.contains(leftKey)) {
        dx -= speed;
    }
    if (keys.contains(rightKey)) {
        dx += speed;
    }

    if (dx > 0) {
        facingRight = true;
    } else if (dx < 0) {
        facingRight = false;
    }

    if (spriteItem) {
        if (facingRight) {
            spriteItem->setTransform(QTransform());
            spriteItem->setPos(0, 0);
        } else {
            QTransform t;
            t.scale(-1.0, 1.0);
            spriteItem->setTransform(t);
            spriteItem->setPos(rect().width(), 0);
        }
    }

    // 只在按下瞬间触发跳跃，避免长按连跳
    const bool jumpPressed = keys.contains(jumpKey);
    if (jumpPressed && !jumpPressedLastFrame && isOnGround) {
        velocityY = -jumpSpeed;
        isOnGround = false;
    }
    jumpPressedLastFrame = jumpPressed;

    // 每帧施加重力
    velocityY += gravity;

    // 保存移动前包围盒，用于判断碰撞来源方向
    const QRectF oldRect = sceneBoundingRect();

    // 1) 水平移动：地板与平台都按实体侧碰处理
    setX(x() + dx);
    const QList<QGraphicsItem*> horizontalHits = collidingItems();
    for (QGraphicsItem* item : horizontalHits) {
        Ground* ground = qgraphicsitem_cast<Ground*>(item);
        Platform* platform = qgraphicsitem_cast<Platform*>(item);
        if (!ground && !platform) {
            continue;
        }

        const QRectF playerNow = sceneBoundingRect();
        const QRectF blockRect = item->sceneBoundingRect();
        const QRectF overlap = playerNow.intersected(blockRect);
        if (overlap.isEmpty() || overlap.height() <= kEpsilon || overlap.width() <= kEpsilon) {
            continue;
        }

        if (dx > 0 && oldRect.right() <= blockRect.left() + kEpsilon) {
            setX(blockRect.left() - rect().width());
        } else if (dx < 0 && oldRect.left() >= blockRect.right() - kEpsilon) {
            setX(blockRect.right());
        }
    }

    const QRectF beforeVerticalRect = sceneBoundingRect();
    const qreal oldTop = beforeVerticalRect.top();
    const qreal oldBottom = beforeVerticalRect.bottom();

    // 2) 垂直移动：地板是实心，平台只允许从上方落下时站上去
    setY(y() + velocityY);
    isOnGround = false;
    const QList<QGraphicsItem*> verticalHits = collidingItems();
    for (QGraphicsItem* item : verticalHits) {
        Ground* ground = qgraphicsitem_cast<Ground*>(item);
        Platform* platform = qgraphicsitem_cast<Platform*>(item);
        if (!ground && !platform) {
            continue;
        }

        const QRectF playerNow = sceneBoundingRect();
        const QRectF blockRect = item->sceneBoundingRect();
        const QRectF overlap = playerNow.intersected(blockRect);
        if (overlap.isEmpty() || overlap.height() <= kEpsilon || overlap.width() <= kEpsilon) {
            continue;
        }

        const qreal newTop = playerNow.top();
        const qreal newBottom = playerNow.bottom();
        const bool crossesTop = oldBottom <= blockRect.top() + kEpsilon
                                && newBottom >= blockRect.top() - kEpsilon;
        const bool crossesBottom = oldTop >= blockRect.bottom() - kEpsilon
                                   && newTop <= blockRect.bottom() + kEpsilon;
        const bool entersFromAbove = oldTop < blockRect.top() - kEpsilon
                                     && newBottom > blockRect.top() + kEpsilon;
        const bool landingFromAbove = crossesTop || entersFromAbove;

        if (ground) {
            // 地板：上下都阻挡，角色可稳定站立
            if (velocityY >= 0 && landingFromAbove) {
                setY(blockRect.top() - rect().height());
                velocityY = 0.0;
                isOnGround = true;
            } else if (velocityY < 0 && crossesBottom) {
                setY(blockRect.bottom());
                velocityY = 0.0;
            }
            continue;
        }

        // 平台：只在从上方向下落时可站立
        if (platform && velocityY >= 0 && landingFromAbove) {
            setY(blockRect.top() - rect().height());
            velocityY = 0.0;
            isOnGround = true;
        }
    }

    // 场景边界限制，防止角色走出地图
    if (scene()) {
        const QRectF bounds = scene()->sceneRect();
        const qreal minX = bounds.left();
        const qreal maxX = bounds.right() - rect().width();
        const qreal minY = bounds.top();

        if (x() < minX) {
            setX(minX);
        } else if (x() > maxX) {
            setX(maxX);
        }

        if (y() < minY) {
            setY(minY);
            velocityY = 0.0;
        }
    }
}
