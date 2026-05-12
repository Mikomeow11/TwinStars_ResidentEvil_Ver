#ifndef GAMESCENE_H
#define GAMESCENE_H

#include "Player.h"
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QQueue>
#include <QSet>
#include <QTimer>
#include <QVector>

class GameScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit GameScene(QObject *parent = nullptr);
    void pauseGame();
    void resumeGame();
    void stopGame();
    void restartCurrentLevel();

signals:
    void gameCompleted(int herbsCollected, int totalHerbs, bool bearKeychainCollected, int deathCount);
    void pauseRequested();

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
    struct ItemTrigger {
        QRectF area;
        QString itemType;
        QString p1PickupText;
        QString p2PickupText;
        bool picked = false;
        QGraphicsItem* visualItem = nullptr;
    };
    struct SwitchTrigger {
        QRectF area;
        QString triggerDialogue;
        bool triggered = false;
        QGraphicsRectItem* visualItem = nullptr;
    };

    struct LevelData {
        QSizeF mapSize;
        QPointF p1Spawn;
        QPointF p2Spawn;
        QString openingDialogue;
        QVector<RectBlock> grounds;
        QVector<RectBlock> platforms;
        QVector<RectBlock> spikes;
        QRectF goalRect;
        QVector<DialogueTrigger> dialogues;
        QVector<ItemTrigger> items;
        QVector<SwitchTrigger> switches;
        QVector<RectBlock> doors;
        bool chaseWallEnabled = false;
        QRectF chaseWallRect;
        qreal chaseWallSpeed = 0.0;
    };

    void initLevels();
    void loadLevel(int levelIndex);
    void updateCamera();
    void updateHud();
    void checkGoalAndMaybeSwitchLevel();
    void checkDialogueTriggers();
    void checkSpikeCollisionAndRespawn();
    void checkItemPickups();
    void updateLevelMechanics();
    void checkIdleDialogue();
    void showDialogue(const QString& text, int frames);
    void displayDialogueNow(const QString& text, int frames);
    int adjustedDialogueFrames(const QString& text, int frames) const;
    bool isSystemDialogue(const QString& text) const;
    QString randomDialogue(const QVector<QString>& dialogues) const;
    int countUntriggeredDialogues(const LevelData& lv) const;

    Player *p1;
    Player *p2;
    QTimer *gameTimer;
    QSet<int> keys;

    QVector<LevelData> levels;
    int currentLevelIndex;

    QVector<QGraphicsItem*> levelItems;
    QGraphicsRectItem *goalItem;
    QVector<QGraphicsItem*> activeDoorItems;
    QGraphicsRectItem *chaseWallItem;
    bool chaseWallIntroShown;

    QGraphicsRectItem *dialogueBoxItem;
    QGraphicsTextItem *dialogueTextItem;
    QGraphicsRectItem *hudBoxItem;
    QGraphicsTextItem *hudTextItem;
    int dialogueFramesLeft;
    QQueue<QPair<QString, int>> dialogueQueue;
    int goalLockHintCooldownFrames;
    QString pendingNextLevelDialogue;

    bool p1ReachedGoal;
    bool p2ReachedGoal;
    bool firstArrivalDialogueDone;
    bool secondArrivalDialogueDone;
    int totalHerbs;
    int herbsCollected;
    int deathCount;
    bool bearKeychainCollected;

    QPointF p1LastPos;
    QPointF p2LastPos;
    int p1IdleFrames;
    int p2IdleFrames;
    bool p1IdleHintUsed;
    bool p2IdleHintUsed;
};

#endif // GAMESCENE_H
