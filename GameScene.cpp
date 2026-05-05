#include "GameScene.h"

#include "Ground.h"
#include "Platform.h"
#include "Spike.h"
#include <QGraphicsView>
#include <QKeyEvent>
#include <QPen>
#include <QPixmap>
#include <utility>

GameScene::GameScene(QObject *parent)
    : QGraphicsScene(parent),
      p1(new Player(1, Qt::cyan)),
      p2(new Player(2, Qt::magenta)),
      gameTimer(new QTimer(this)),
      currentLevelIndex(0),
      goalItem(nullptr),
      dialogueTextItem(new QGraphicsTextItem()),
      dialogueFramesLeft(0),
      goalLockHintCooldownFrames(0)
{
    addItem(p1);
    addItem(p2);

    dialogueTextItem->setDefaultTextColor(Qt::white);
    dialogueTextItem->setZValue(2000);
    dialogueTextItem->setVisible(false);
    addItem(dialogueTextItem);

    initLevels();
    loadLevel(0);

    connect(gameTimer, &QTimer::timeout, this, &GameScene::mainGameLoop);
    gameTimer->start(16);
}

void GameScene::initLevels() {
    levels.clear();

    LevelData lv1;
    lv1.mapSize = QSizeF(2200, 600);
    lv1.p1Spawn = QPointF(100, 500);
    lv1.p2Spawn = QPointF(220, 500);
    lv1.grounds = {
        { QRectF(0, 560, 2200, 40) }
    };
    lv1.platforms = {
        { QRectF(320, 470, 180, 20) },
        { QRectF(620, 420, 220, 20) },
        { QRectF(980, 360, 180, 20) },
        { QRectF(1320, 440, 220, 20) },
        { QRectF(1720, 380, 180, 20) }
    };
    lv1.goalRect = QRectF(2060, 480, 60, 80);
    lv1.dialogues = {
        // Trigger 1: must jump onto platform at x=620, y=420, w=220, h=20
        { QRectF(620, 370, 220, 50), QString("Watch your jump timing"), false },
        // Trigger 2: must jump onto platform at x=1320, y=440, w=220, h=20
        { QRectF(1320, 390, 220, 50), QString("Almost at the goal"), false }
    };
    lv1.spikes = {};
    levels.push_back(lv1);

    LevelData lv2;
    lv2.mapSize = QSizeF(2600, 600);
    lv2.p1Spawn = QPointF(80, 500);
    lv2.p2Spawn = QPointF(180, 500);
    lv2.grounds = {
        { QRectF(0, 560, 2600, 40) }
    };
    lv2.platforms = {
        { QRectF(300, 500, 140, 20) },
        { QRectF(520, 450, 140, 20) },
        { QRectF(740, 400, 140, 20) },
        { QRectF(960, 350, 140, 20) },
        { QRectF(1240, 430, 180, 20) },
        { QRectF(1540, 380, 200, 20) },
        { QRectF(1880, 330, 180, 20) },
        { QRectF(2200, 420, 180, 20) }
    };
    // Second level spikes: placed on ground, can be jumped over.
    lv2.spikes = {
        { QRectF(430, 530, 36, 30) },
        { QRectF(466, 530, 36, 30) },
        { QRectF(502, 530, 36, 30) },
        { QRectF(1420, 530, 36, 30) },
        { QRectF(1456, 530, 36, 30) },
        { QRectF(1492, 530, 36, 30) },
        { QRectF(2080, 530, 36, 30) },
        { QRectF(2116, 530, 36, 30) }
    };
    lv2.goalRect = QRectF(2460, 480, 60, 80);
    lv2.dialogues = {
        { QRectF(680, 0, 120, 600), QString("Chain your jumps here"), false },
        { QRectF(1960, 0, 120, 600), QString("Final stretch"), false }
    };
    levels.push_back(lv2);
}

void GameScene::loadLevel(int levelIndex) {
    if (levels.isEmpty()) {
        return;
    }

    currentLevelIndex = (levelIndex % levels.size() + levels.size()) % levels.size();
    LevelData &lv = levels[currentLevelIndex];

    keys.clear();
    dialogueFramesLeft = 0;
    goalLockHintCooldownFrames = 0;
    dialogueTextItem->setVisible(false);

    for (QGraphicsItem* item : std::as_const(levelItems)) {
        removeItem(item);
        delete item;
    }
    levelItems.clear();
    goalItem = nullptr;

    setSceneRect(0, 0, lv.mapSize.width(), lv.mapSize.height());

    // 第一关背景图（2200x600），放在最底层。
    if (currentLevelIndex == 0) {
        QPixmap level1Bg(":/assets/backgrouds/level1_bg.png");
        if (!level1Bg.isNull()) {
            auto *bgItem = new QGraphicsPixmapItem(level1Bg);
            bgItem->setPos(0, 0);
            bgItem->setZValue(-1000);
            levelItems.push_back(bgItem);
            addItem(bgItem);
        } else {
            showDialogue(QString("Background load failed: :/assets/backgrouds/level1_bg.png"), 180);
        }
    }

    for (const RectBlock &g : std::as_const(lv.grounds)) {
        auto *ground = new Ground(g.rect.x(), g.rect.y(), g.rect.width(), g.rect.height());
        levelItems.push_back(ground);
        addItem(ground);
    }

    for (const RectBlock &p : std::as_const(lv.platforms)) {
        auto *platform = new Platform(p.rect.x(), p.rect.y(), p.rect.width(), p.rect.height());
        levelItems.push_back(platform);
        addItem(platform);
    }

    for (const RectBlock &s : std::as_const(lv.spikes)) {
        auto *spike = new Spike(s.rect.x(), s.rect.y(), s.rect.width(), s.rect.height());
        spike->setZValue(20);
        levelItems.push_back(spike);
        addItem(spike);
    }

    goalItem = new QGraphicsRectItem(lv.goalRect);
    goalItem->setBrush(QColor(100, 220, 120, 180));
    goalItem->setPen(QPen(Qt::NoPen));
    goalItem->setZValue(10);
    levelItems.push_back(goalItem);
    addItem(goalItem);

    for (DialogueTrigger &d : lv.dialogues) {
        d.triggered = false;
    }

    p1->setPos(lv.p1Spawn);
    p2->setPos(lv.p2Spawn);

    showDialogue(QString("Level %1").arg(currentLevelIndex + 1), 90);
}

void GameScene::keyPressEvent(QKeyEvent *event) {
    keys.insert(event->key());
}

void GameScene::keyReleaseEvent(QKeyEvent *event) {
    keys.remove(event->key());
}

void GameScene::mainGameLoop() {
    p1->updatePosition(keys);
    p2->updatePosition(keys);

    checkGoalAndMaybeSwitchLevel();
    checkDialogueTriggers();
    checkSpikeCollisionAndRespawn();
    updateCamera();

    if (dialogueFramesLeft > 0) {
        --dialogueFramesLeft;
        if (dialogueFramesLeft == 0) {
            dialogueTextItem->setVisible(false);
        }
    }

    if (goalLockHintCooldownFrames > 0) {
        --goalLockHintCooldownFrames;
    }
}

void GameScene::checkGoalAndMaybeSwitchLevel() {
    if (!goalItem) {
        return;
    }

    if (!p1->sceneBoundingRect().intersects(goalItem->sceneBoundingRect())) {
        return;
    }

    LevelData &lv = levels[currentLevelIndex];

    // 第一关：必须触发全部对话点才能过关。
    if (currentLevelIndex == 0) {
        const int remaining = countUntriggeredDialogues(lv);
        if (remaining > 0) {
            if (goalLockHintCooldownFrames == 0) {
                showDialogue(QString("Need %1 more dialogue trigger(s)").arg(remaining), 90);
                goalLockHintCooldownFrames = 75;
            }
            return;
        }
    }

    loadLevel(currentLevelIndex + 1);
}

void GameScene::checkDialogueTriggers() {
    LevelData &lv = levels[currentLevelIndex];
    const QRectF p1Rect = p1->sceneBoundingRect();

    for (DialogueTrigger &d : lv.dialogues) {
        if (d.triggered) {
            continue;
        }
        if (p1Rect.intersects(d.area)) {
            d.triggered = true;
            showDialogue(d.text, 160);
            break;
        }
    }
}

void GameScene::showDialogue(const QString& text, int frames) {
    dialogueTextItem->setPlainText(text);
    dialogueTextItem->setVisible(true);
    dialogueFramesLeft = frames;
}

void GameScene::checkSpikeCollisionAndRespawn() {
    LevelData &lv = levels[currentLevelIndex];

    bool p1Hit = false;
    bool p2Hit = false;

    const auto p1Hits = p1->collidingItems();
    for (QGraphicsItem* item : p1Hits) {
        if (qgraphicsitem_cast<Spike*>(item)) {
            p1Hit = true;
            break;
        }
    }

    const auto p2Hits = p2->collidingItems();
    for (QGraphicsItem* item : p2Hits) {
        if (qgraphicsitem_cast<Spike*>(item)) {
            p2Hit = true;
            break;
        }
    }

    if (!p1Hit && !p2Hit) {
        return;
    }

    if (p1Hit) {
        p1->setPos(lv.p1Spawn);
    }
    if (p2Hit) {
        p2->setPos(lv.p2Spawn);
    }

    if (p1Hit && p2Hit) {
        showDialogue(QString("Both players hit spikes!"), 90);
    } else if (p1Hit) {
        showDialogue(QString("Player 1 hit spike!"), 90);
    } else {
        showDialogue(QString("Player 2 hit spike!"), 90);
    }
}

int GameScene::countUntriggeredDialogues(const LevelData& lv) const {
    int remaining = 0;
    for (const DialogueTrigger &d : lv.dialogues) {
        if (!d.triggered) {
            ++remaining;
        }
    }
    return remaining;
}

void GameScene::updateCamera() {
    const auto sceneViews = views();
    if (sceneViews.isEmpty()) {
        return;
    }

    QGraphicsView *view = sceneViews.first();
    const QRectF viewRect = view->viewport()->rect();
    const qreal halfW = viewRect.width() * 0.5;
    const qreal halfH = viewRect.height() * 0.5;

    qreal cx = p1->x() + p1->rect().width() * 0.5;
    qreal cy = p1->y() + p1->rect().height() * 0.5;

    const QRectF s = sceneRect();
    if (s.width() > viewRect.width()) {
        cx = qBound(s.left() + halfW, cx, s.right() - halfW);
    } else {
        cx = s.center().x();
    }

    if (s.height() > viewRect.height()) {
        cy = qBound(s.top() + halfH, cy, s.bottom() - halfH);
    } else {
        cy = s.center().y();
    }

    view->centerOn(cx, cy);

    if (dialogueTextItem->isVisible()) {
        dialogueTextItem->setPos(cx - halfW + 20, cy - halfH + 16);
    }
}
