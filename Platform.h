#ifndef PLATFORM_H
#define PLATFORM_H

#include <QBrush>
#include <QGraphicsRectItem>

class Platform : public QGraphicsRectItem {
public:
    explicit Platform(qreal x, qreal y, qreal w, qreal h, QGraphicsItem* parent = nullptr);

    // 让 qgraphicsitem_cast<Platform*>() 能正确识别该类型。
    enum { Type = UserType + 1 };
    int type() const override { return Type; }
};

#endif // PLATFORM_H
