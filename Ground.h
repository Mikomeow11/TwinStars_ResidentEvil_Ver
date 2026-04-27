#ifndef GROUND_H
#define GROUND_H

#include <QBrush>
#include <QGraphicsRectItem>

class Ground : public QGraphicsRectItem {
public:
    explicit Ground(qreal x, qreal y, qreal w, qreal h, QGraphicsItem* parent = nullptr);

    // 预留：地板可配置危险伤害值（例如尖刺地面）
    void setHazardDamage(int damage) { hazardDamage = damage; }
    int getHazardDamage() const { return hazardDamage; }

    enum { Type = UserType + 2 };
    int type() const override { return Type; }

private:
    int hazardDamage;
};

#endif // GROUND_H
