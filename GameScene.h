#ifndef GAMESCENE_H
#define GAMESCENE_H

#include "Player.h"
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QSet>
#include <QTimer>
#include <QVector>

class GameScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit GameScene(QObject *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void mainGameLoop();

private:
    struct RectBlock {
        QRectF rect;
    };

    struct DialogueTrigger {
        QRectF area;
        QString text;
        bool triggered = false;
    };

    struct LevelData {
        QSizeF mapSize;
        QPointF p1Spawn;
        QPointF p2Spawn;
        QVector<RectBlock> grounds;
        QVector<RectBlock> platforms;
        QVector<RectBlock> spikes;
        QRectF goalRect;
        QVector<DialogueTrigger> dialogues;
    };

    void initLevels();
    void loadLevel(int levelIndex);
    void updateCamera();
    void checkGoalAndMaybeSwitchLevel();
    void checkDialogueTriggers();
    void checkSpikeCollisionAndRespawn();
    void showDialogue(const QString& text, int frames);
    int countUntriggeredDialogues(const LevelData& lv) const;

    Player *p1;
    Player *p2;
    QTimer *gameTimer;
    QSet<int> keys;

    QVector<LevelData> levels;
    int currentLevelIndex;

    QVector<QGraphicsItem*> levelItems;
    QGraphicsRectItem *goalItem;

    QGraphicsTextItem *dialogueTextItem;
    int dialogueFramesLeft;
    int goalLockHintCooldownFrames;
};

#endif // GAMESCENE_H
