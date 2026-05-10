#include "GameScene.h"

#include "Ground.h"
#include "Platform.h"
#include "Spike.h"
#include <QGraphicsView>
#include <QKeyEvent>
#include <QPen>
#include <QPixmap>
#include <QRandomGenerator>
#include <QStringList>
#include <QTextDocument>
#include <QTextOption>
#include <utility>

GameScene::GameScene(QObject *parent)
    : QGraphicsScene(parent),
      p1(new Player(1, Qt::cyan)),
      p2(new Player(2, Qt::magenta)),
      gameTimer(new QTimer(this)),
      currentLevelIndex(0),
      goalItem(nullptr),
      chaseWallItem(nullptr),
      chaseWallIntroShown(false),
      dialogueBoxItem(new QGraphicsRectItem()),
      dialogueTextItem(new QGraphicsTextItem()),
      dialogueFramesLeft(0),
      goalLockHintCooldownFrames(0),
      pendingNextLevelDialogue(""),
      p1ReachedGoal(false),
      p2ReachedGoal(false),
      firstArrivalDialogueDone(false),
      secondArrivalDialogueDone(false),
      totalHerbs(0),
      herbsCollected(0),
      deathCount(0),
      bearKeychainCollected(false),
      p1IdleFrames(0),
      p2IdleFrames(0),
      p1IdleHintUsed(false),
      p2IdleHintUsed(false)
{
    addItem(p1);
    addItem(p2);

    dialogueBoxItem->setBrush(QColor(0, 0, 0, 175));
    dialogueBoxItem->setPen(QPen(QColor("#df9fb6"), 2));
    dialogueBoxItem->setZValue(1999);
    dialogueBoxItem->setVisible(false);
    addItem(dialogueBoxItem);

    dialogueTextItem->setDefaultTextColor(QColor("#fff4b8"));
    dialogueTextItem->setTextWidth(680);
    dialogueTextItem->document()->setDefaultTextOption(QTextOption(Qt::AlignLeft));
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
    totalHerbs = 0;
    herbsCollected = 0;
    deathCount = 0;
    bearKeychainCollected = false;

    LevelData lv1;
    lv1.mapSize = QSizeF(2200, 600);
    lv1.p1Spawn = QPointF(100, 500);
    lv1.p2Spawn = QPointF(220, 500);
    lv1.openingDialogue = QString::fromUtf8("里昂：这不是普通实验室，小心点。");
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
        // 第一处平台提示：玩家跳到该平台附近时触发。
        { QRectF(620, 370, 220, 50), QString::fromUtf8("艾达：跳之前先看落点。"), false },
        // 第二处平台提示：提醒玩家后续平台不稳定。
        { QRectF(1320, 390, 220, 50), QString::fromUtf8("里昂：这些平台不太稳，别急着冲。"), false }
    };
    lv1.spikes = {};
    lv1.items = {
        { QRectF(980, 320, 36, 36), QString("herb"), QString::fromUtf8("里昂：草药，这可是好东西。"), QString::fromUtf8("艾达：草药吗？以备不时之需吧。"), false, nullptr },
        { QRectF(1720, 340, 36, 36), QString("herb"), QString::fromUtf8("里昂：这儿绿化做的不错。"), QString::fromUtf8("艾达：草药，别浪费了。"), false, nullptr }
    };
    levels.push_back(lv1);

    LevelData lv2;
    lv2.mapSize = QSizeF(2600, 600);
    lv2.p1Spawn = QPointF(80, 500);
    lv2.p2Spawn = QPointF(180, 500);
    lv2.openingDialogue = QString::fromUtf8("艾达：地面有机关，别只顾着往前冲。");
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
    // 尖刺：放在地上可以被跳过
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
        { QRectF(380, 0, 160, 600), QString::fromUtf8("里昂：这些尖刺不是摆设。"), false },
        { QRectF(680, 0, 120, 600), QString::fromUtf8("艾达：跳之前先看落点。"), false },
        { QRectF(1960, 0, 120, 600), QString::fromUtf8("里昂：后半段更窄，稳一点。"), false }
    };
    lv2.items = {
        { QRectF(960, 315, 36, 36), QString("herb"), QString::fromUtf8("里昂：草药，拿着吧。"), QString::fromUtf8("艾达：草药，收好了。"), false, nullptr },
        { QRectF(1880, 295, 36, 36), QString("herb"), QString::fromUtf8("里昂：反正我不嫌多。"), QString::fromUtf8("艾达：不错，继续前进。"), false, nullptr }
    };
    levels.push_back(lv2);

    LevelData lv3;
    lv3.mapSize = QSizeF(3400, 600);
    lv3.p1Spawn = QPointF(460, 500);
    lv3.p2Spawn = QPointF(560, 500);
    lv3.openingDialogue = QString::fromUtf8("里昂：看来这门需要两边同时解锁。");
    lv3.grounds = {
        { QRectF(0, 560, 3400, 40) }
    };
    lv3.platforms = {
        { QRectF(310, 500, 110, 20) },
        { QRectF(160, 455, 110, 20) },
        { QRectF(20, 405, 110, 20) },
        { QRectF(620, 485, 180, 20) },
        { QRectF(620, 420, 180, 20) },
        { QRectF(940, 360, 190, 20) },
        { QRectF(1260, 300, 180, 20) },
        { QRectF(1580, 355, 180, 20) },
        { QRectF(1820, 500, 220, 20) },
        { QRectF(2140, 440, 220, 20) },
        { QRectF(2480, 390, 180, 20) },
        { QRectF(2820, 455, 200, 20) }
    };
    lv3.spikes = {
        { QRectF(760, 530, 36, 30) },
        { QRectF(796, 530, 36, 30) },
        { QRectF(2040, 530, 36, 30) },
        { QRectF(2076, 530, 36, 30) }
    };
    lv3.goalRect = QRectF(3260, 480, 60, 80);
    lv3.dialogues = {
        { QRectF(300, 0, 120, 600), QString::fromUtf8("艾达：往回走？你发现什么了？"), false },
        { QRectF(520, 0, 120, 600), QString::fromUtf8("艾达：前面分路，别跟丢了。"), false },
        { QRectF(1280, 0, 180, 600), QString::fromUtf8("系统：检测到双人权限缺失。"), false },
        { QRectF(2120, 0, 160, 600), QString::fromUtf8("艾达：门锁在另一边。"), false },
        { QRectF(2720, 0, 120, 600), QString::fromUtf8("里昂：出口就在前面，先把门打开。"), false }
    };
    lv3.items = {
        { QRectF(56, 365, 36, 36), QString("bear_keychain"), QString::fromUtf8("艾达：看来某人旧情难忘啊。\n里昂：我只是习惯把重要的东西带在身上。"), QString(), false, nullptr },
        { QRectF(1580, 315, 36, 36), QString("herb"), QString::fromUtf8("里昂：这里还有补给。"), QString::fromUtf8("艾达：拿上，后面用得着。"), false, nullptr }
    };
    lv3.switches = {
        { QRectF(1320, 260, 64, 28), QString::fromUtf8("里昂：这边的门锁松动了。"), false, nullptr },
        { QRectF(2200, 400, 64, 28), QString::fromUtf8("艾达：别急，我找到控制台了。"), false, nullptr }
    };
    lv3.doors = {
        { QRectF(3100, 360, 52, 200) }
    };
    levels.push_back(lv3);

    LevelData lv4;
    lv4.mapSize = QSizeF(3200, 600);
    lv4.p1Spawn = QPointF(110, 500);
    lv4.p2Spawn = QPointF(210, 500);
    lv4.openingDialogue = QString::fromUtf8("系统：自毁程序已启动。");
    lv4.grounds = {
        { QRectF(0, 560, 3200, 40) }
    };
    lv4.platforms = {
        { QRectF(360, 500, 160, 20) },
        { QRectF(620, 445, 170, 20) },
        { QRectF(920, 390, 170, 20) },
        { QRectF(1240, 455, 190, 20) },
        { QRectF(1580, 400, 190, 20) },
        { QRectF(1940, 350, 180, 20) },
        { QRectF(2260, 430, 200, 20) },
        { QRectF(2620, 380, 190, 20) }
    };
    lv4.spikes = {
        { QRectF(780, 530, 36, 30) },
        { QRectF(816, 530, 36, 30) },
        { QRectF(1500, 530, 36, 30) },
        { QRectF(1536, 530, 36, 30) },
        { QRectF(2460, 530, 36, 30) },
        { QRectF(2496, 530, 36, 30) }
    };
    lv4.goalRect = QRectF(3060, 480, 60, 80);
    lv4.dialogues = {
        { QRectF(220, 0, 120, 600), QString::fromUtf8("艾达：别停，后面的东西追上来就麻烦了。"), false },
        { QRectF(740, 0, 140, 600), QString::fromUtf8("里昂：这些尖刺不是摆设。"), false },
        { QRectF(1220, 0, 140, 600), QString::fromUtf8("艾达：跳之前先看落点。"), false },
        { QRectF(1760, 0, 120, 600), QString::fromUtf8("里昂：快走，后面撑不住了！"), false }
    };
    lv4.items = {
        { QRectF(1240, 415, 36, 36), QString("herb"), QString::fromUtf8("里昂：最后的补给了。"), QString::fromUtf8("艾达：收下，别回头。"), false, nullptr }
    };
    lv4.chaseWallEnabled = true;
    lv4.chaseWallRect = QRectF(-180, 0, 90, 600);
    lv4.chaseWallSpeed = 1.35;
    levels.push_back(lv4);

    for (const LevelData &level : std::as_const(levels)) {
        for (const ItemTrigger &item : level.items) {
            if (item.itemType == QString("herb")) {
                ++totalHerbs;
            }
        }
    }
}

void GameScene::loadLevel(int levelIndex) {
    if (levels.isEmpty()) {
        return;
    }

    currentLevelIndex = (levelIndex % levels.size() + levels.size()) % levels.size();
    LevelData &lv = levels[currentLevelIndex];

    keys.clear();
    dialogueFramesLeft = 0;
    dialogueQueue.clear();
    goalLockHintCooldownFrames = 0;
    p1ReachedGoal = false;
    p2ReachedGoal = false;
    firstArrivalDialogueDone = false;
    secondArrivalDialogueDone = false;
    p1IdleFrames = 0;
    p2IdleFrames = 0;
    p1IdleHintUsed = false;
    p2IdleHintUsed = false;
    dialogueBoxItem->setVisible(false);
    dialogueTextItem->setVisible(false);

    for (QGraphicsItem* item : std::as_const(levelItems)) {
        removeItem(item);
        delete item;
    }
    levelItems.clear();
    activeDoorItems.clear();
    goalItem = nullptr;
    chaseWallItem = nullptr;
    chaseWallIntroShown = false;

    setSceneRect(0, 0, lv.mapSize.width(), lv.mapSize.height());

    QString backgroundPath;
    if (currentLevelIndex == 0) {
        backgroundPath = ":/assets/backgrouds/level1_bg.png";
    } else if (currentLevelIndex == 1) {
        backgroundPath = ":/assets/backgrouds/level2_bg.png";
    } else if (currentLevelIndex == 2) {
        backgroundPath = ":/assets/backgrouds/level3_bg.png";
    } else if (currentLevelIndex == 3) {
        backgroundPath = ":/assets/backgrouds/level4_bg.png";
    }

    if (!backgroundPath.isEmpty()) {
        QPixmap levelBg(backgroundPath);
        if (!levelBg.isNull()) {
            auto *bgItem = new QGraphicsPixmapItem(levelBg);
            bgItem->setPos(0, 0);
            bgItem->setZValue(-1000);
            levelItems.push_back(bgItem);
            addItem(bgItem);
        } else {
            showDialogue(QString::fromUtf8("背景加载失败：%1").arg(backgroundPath), 180);
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

    for (ItemTrigger &it : lv.items) {
        it.picked = false;
        it.visualItem = nullptr;
        const bool isBearKeychain = it.itemType == QString("bear_keychain");
        QPixmap itemPixmap(isBearKeychain ? QString(":/assets/items/bear_keychain.png") : QString(":/assets/items/herb.png"));
        QGraphicsItem* itemVisual = nullptr;
        if (!itemPixmap.isNull()) {
            auto *pixmapItem = new QGraphicsPixmapItem(
                itemPixmap.scaled(it.area.size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation)
            );
            const QRectF itemBounds = pixmapItem->boundingRect();
            pixmapItem->setPos(
                it.area.x() + (it.area.width() - itemBounds.width()) * 0.5,
                it.area.y() + (it.area.height() - itemBounds.height()) * 0.5
            );
            pixmapItem->setZValue(30);
            itemVisual = pixmapItem;
        } else {
            auto *itemRect = new QGraphicsRectItem(it.area);
            itemRect->setBrush(isBearKeychain ? QColor(255, 190, 80, 220) : QColor(255, 220, 80, 200));
            itemRect->setPen(isBearKeychain ? QPen(QColor(255, 245, 170), 2) : QPen(Qt::NoPen));
            itemRect->setZValue(30);
            itemVisual = itemRect;
        }
        it.visualItem = itemVisual;
        levelItems.push_back(itemVisual);
        addItem(itemVisual);
    }

    for (SwitchTrigger &sw : lv.switches) {
        sw.triggered = false;
        auto *switchItem = new QGraphicsRectItem(sw.area);
        switchItem->setBrush(QColor(80, 210, 230, 210));
        switchItem->setPen(QPen(QColor(220, 255, 255), 2));
        switchItem->setZValue(25);
        sw.visualItem = switchItem;
        levelItems.push_back(switchItem);
        addItem(switchItem);
    }

    for (const RectBlock &door : std::as_const(lv.doors)) {
        auto *doorItem = new Platform(door.rect.x(), door.rect.y(), door.rect.width(), door.rect.height());
        doorItem->setBrush(QColor(80, 30, 120, 220));
        doorItem->setPen(QPen(QColor(255, 230, 120), 3));
        doorItem->setZValue(35);
        activeDoorItems.push_back(doorItem);
        levelItems.push_back(doorItem);
        addItem(doorItem);
    }

    if (lv.chaseWallEnabled) {
        chaseWallItem = new QGraphicsRectItem(lv.chaseWallRect);
        chaseWallItem->setBrush(QColor(220, 20, 45, 120));
        chaseWallItem->setPen(QPen(QColor(255, 120, 120), 3));
        chaseWallItem->setZValue(40);
        levelItems.push_back(chaseWallItem);
        addItem(chaseWallItem);
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
    p1LastPos = p1->pos();
    p2LastPos = p2->pos();

    if (!pendingNextLevelDialogue.isEmpty()) {
        showDialogue(pendingNextLevelDialogue, 120);
        pendingNextLevelDialogue.clear();
    } else if (!lv.openingDialogue.isEmpty()) {
        showDialogue(lv.openingDialogue, 140);
    }
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
    checkItemPickups();
    updateLevelMechanics();
    checkSpikeCollisionAndRespawn();
    checkIdleDialogue();
    updateCamera();

    if (dialogueFramesLeft > 0) {
        --dialogueFramesLeft;
        if (dialogueFramesLeft == 0) {
            if (!dialogueQueue.isEmpty()) {
                const auto next = dialogueQueue.dequeue();
                displayDialogueNow(next.first, next.second);
            } else {
                dialogueBoxItem->setVisible(false);
                dialogueTextItem->setVisible(false);
            }
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

    const bool p1AtGoal = p1->sceneBoundingRect().intersects(goalItem->sceneBoundingRect());
    const bool p2AtGoal = p2->sceneBoundingRect().intersects(goalItem->sceneBoundingRect());

    if (!p1AtGoal && !p2AtGoal) {
        return;
    }

    LevelData &lv = levels[currentLevelIndex];

    // 第一关：必须触发全部对话点才能过关。
    if (currentLevelIndex == 0) {
        const int remaining = countUntriggeredDialogues(lv);
        if (remaining > 0) {
            if (goalLockHintCooldownFrames == 0) {
                showDialogue(QString("里昂：似乎还有地方没有调查过，回头看看吧。").arg(remaining), 90);
                goalLockHintCooldownFrames = 75;
            }
            return;
        }
    }

    if (p1AtGoal && p2AtGoal && !p1ReachedGoal && !p2ReachedGoal) {
        p1ReachedGoal = true;
        p2ReachedGoal = true;
        if (currentLevelIndex == levels.size() - 1) {
            gameTimer->stop();
            emit gameCompleted(herbsCollected, totalHerbs, bearKeychainCollected, deathCount);
            return;
        }
        pendingNextLevelDialogue = QString::fromUtf8("里昂：配合不错。\n艾达：别停，下一段才麻烦。");
        loadLevel(currentLevelIndex + 1);
        return;
    }

    if (p1AtGoal && !p1ReachedGoal) {
        p1ReachedGoal = true;
        if (currentLevelIndex == 0) {
            if (!firstArrivalDialogueDone) {
                showDialogue(QString::fromUtf8("艾达？"), 100);
                firstArrivalDialogueDone = true;
            } else if (!secondArrivalDialogueDone) {
                pendingNextLevelDialogue = QString::fromUtf8("这不是跟上了吗？");
                secondArrivalDialogueDone = true;
            }
        } else {
            if (!firstArrivalDialogueDone) {
                showDialogue(QString::fromUtf8("里昂：我先去前面探路。"), 100);
                firstArrivalDialogueDone = true;
            } else if (!secondArrivalDialogueDone) {
                showDialogue(QString::fromUtf8("里昂：跟上了，我们继续。"), 100);
                secondArrivalDialogueDone = true;
            }
        }
    }
    if (p2AtGoal && !p2ReachedGoal) {
        p2ReachedGoal = true;
        if (currentLevelIndex == 0) {
            if (!firstArrivalDialogueDone) {
                showDialogue(QString::fromUtf8("里昂？"), 100);
                firstArrivalDialogueDone = true;
            } else if (!secondArrivalDialogueDone) {
                pendingNextLevelDialogue = QString::fromUtf8("我是在欣赏沿途的风景。");
                secondArrivalDialogueDone = true;
            }
        } else {
            if (!firstArrivalDialogueDone) {
                showDialogue(QString::fromUtf8("艾达：出口我先看过了。"), 100);
                firstArrivalDialogueDone = true;
            } else if (!secondArrivalDialogueDone) {
                showDialogue(QString::fromUtf8("艾达：还不算太慢。"), 100);
                secondArrivalDialogueDone = true;
            }
        }
    }

    if (p1ReachedGoal && p2ReachedGoal) {
        if (currentLevelIndex == levels.size() - 1) {
            gameTimer->stop();
            emit gameCompleted(herbsCollected, totalHerbs, bearKeychainCollected, deathCount);
            return;
        }
        loadLevel(currentLevelIndex + 1);
    }
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
            if (!d.text.isEmpty()) {
                showDialogue(d.text, 160);
            }
            break;
        }
    }
}

void GameScene::showDialogue(const QString& text, int frames) {
    if (text.isEmpty()) {
        return;
    }
    const int finalFrames = adjustedDialogueFrames(text, frames);
    if (dialogueFramesLeft > 0 || dialogueTextItem->isVisible()) {
        if (dialogueQueue.size() < 6) {
            dialogueQueue.enqueue(qMakePair(text, finalFrames));
        }
        return;
    }
    displayDialogueNow(text, finalFrames);
}

void GameScene::displayDialogueNow(const QString& text, int frames) {
    QFont dialogueFont = dialogueTextItem->font();
    dialogueFont.setBold(isSystemDialogue(text));
    dialogueTextItem->setFont(dialogueFont);
    dialogueTextItem->setDefaultTextColor(isSystemDialogue(text) ? QColor("#ff3b30") : QColor("#fff4b8"));
    dialogueTextItem->setPlainText(text);
    dialogueBoxItem->setVisible(true);
    dialogueTextItem->setVisible(true);
    dialogueFramesLeft = frames;
}

int GameScene::adjustedDialogueFrames(const QString& text, int frames) const {
    const int lineCount = text.count('\n') + 1;
    const int lengthBonus = qMin(120, text.size() * 2);
    const int minFrames = lineCount >= 2 ? 220 : 160;
    return qMax(frames + lengthBonus, minFrames);
}

bool GameScene::isSystemDialogue(const QString& text) const {
    const QString trimmed = text.trimmed();
    return trimmed.startsWith(QString::fromUtf8("系统："));
}

QString GameScene::randomDialogue(const QVector<QString>& dialogues) const {
    if (dialogues.isEmpty()) {
        return QString();
    }
    const int index = QRandomGenerator::global()->bounded(dialogues.size());
    return dialogues.at(index);
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
        ++deathCount;
    }
    if (p2Hit) {
        ++deathCount;
    }

    if (p1Hit) {
        p1->setPos(lv.p1Spawn);
    }
    if (p2Hit) {
        p2->setPos(lv.p2Spawn);
    }

    if (p1Hit && p2Hit) {
        const QVector<QString> leonLines = {
            QString::fromUtf8("里昂：相互靠着走，别倒在我前面。"),
            QString::fromUtf8("里昂：还没结束，别再丢下我一个人。"),
            QString::fromUtf8("里昂：你可不像是会轻易认输的人。")
        };
        const QVector<QString> adaLines = {
            QString::fromUtf8("艾达：清醒点，你的命还欠在我手上。"),
            QString::fromUtf8("艾达：别倒下，我可不想白来一趟。"),
            QString::fromUtf8("艾达：你最好不是故意的。")
        };
        showDialogue(randomDialogue(leonLines) + "\n" + randomDialogue(adaLines), 120);
    } else if (p1Hit) {
        showDialogue(randomDialogue({
            QString::fromUtf8("艾达：清醒点，你的命还欠在我手上。"),
            QString::fromUtf8("艾达：别倒下，我可不想白来一趟。"),
            QString::fromUtf8("艾达：你最好不是故意的。")
        }), 120);
    } else {
        showDialogue(randomDialogue({
            QString::fromUtf8("里昂：相互靠着走，别倒在我前面。"),
            QString::fromUtf8("里昂：还没结束，别再丢下我一个人。"),
            QString::fromUtf8("里昂：你可不像是会轻易认输的人。")
        }), 120);
    }
}

void GameScene::checkItemPickups() {
    LevelData &lv = levels[currentLevelIndex];
    const QRectF p1Rect = p1->sceneBoundingRect();
    const QRectF p2Rect = p2->sceneBoundingRect();

    for (ItemTrigger &it : lv.items) {
        if (it.picked) {
            continue;
        }
        const bool p1Pick = p1Rect.intersects(it.area);
        const bool p2Pick = p2Rect.intersects(it.area);
        if (p1Pick || p2Pick) {
            it.picked = true;
            if (it.itemType == QString("herb")) {
                ++herbsCollected;
            } else if (it.itemType == QString("bear_keychain")) {
                bearKeychainCollected = true;
            }
            if (it.visualItem) {
                removeItem(it.visualItem);
                it.visualItem = nullptr;
            }
            if (it.itemType == QString("bear_keychain")) {
                if (!it.p1PickupText.isEmpty()) {
                    const QStringList lines = it.p1PickupText.split('\n', Qt::SkipEmptyParts);
                    for (const QString &line : lines) {
                        showDialogue(line.trimmed(), 140);
                    }
                } else {
                    showDialogue(QString::fromUtf8("小熊钥匙扣：一枚被仔细保存的信物。"), 140);
                }
            } else if (p1Pick) {
                showDialogue(randomDialogue({
                    QString::fromUtf8("里昂：草药，这可是好东西。"),
                    QString::fromUtf8("里昂：留着，后面说不定用得上。"),
                    QString::fromUtf8("里昂：补给不多，先收起来。")
                }), 110);
            } else if (p2Pick) {
                showDialogue(randomDialogue({
                    QString::fromUtf8("艾达：补给不多，别浪费。"),
                    QString::fromUtf8("艾达：看来运气还没完全站在我们对面。"),
                    QString::fromUtf8("艾达：草药，收好了。")
                }), 110);
            } else {
                showDialogue(QString::fromUtf8("拾取道具：%1").arg(it.itemType), 110);
            }
        }
    }
}

void GameScene::updateLevelMechanics() {
    LevelData &lv = levels[currentLevelIndex];
    const QRectF p1Rect = p1->sceneBoundingRect();
    const QRectF p2Rect = p2->sceneBoundingRect();

    bool allSwitchesTriggered = !lv.switches.isEmpty();
    for (SwitchTrigger &sw : lv.switches) {
        if (!sw.triggered && (p1Rect.intersects(sw.area) || p2Rect.intersects(sw.area))) {
            sw.triggered = true;
            if (sw.visualItem) {
                sw.visualItem->setBrush(QColor(120, 255, 120, 220));
                sw.visualItem->setPen(QPen(QColor(245, 255, 180), 2));
            }
            if (!sw.triggerDialogue.isEmpty()) {
                showDialogue(sw.triggerDialogue, 120);
            }
        }

        if (!sw.triggered) {
            allSwitchesTriggered = false;
        }
    }

    if (allSwitchesTriggered && !activeDoorItems.isEmpty()) {
        for (QGraphicsItem* doorItem : std::as_const(activeDoorItems)) {
            removeItem(doorItem);
            levelItems.removeOne(doorItem);
            delete doorItem;
        }
        activeDoorItems.clear();
        showDialogue(QString::fromUtf8("系统：封锁解除。"), 120);
    }

    if (!lv.chaseWallEnabled || !chaseWallItem) {
        return;
    }

    chaseWallItem->moveBy(lv.chaseWallSpeed, 0);
    if (!chaseWallIntroShown && chaseWallItem->sceneBoundingRect().right() > p1->x() - 180) {
        chaseWallIntroShown = true;
        showDialogue(QString::fromUtf8("艾达：这地方要塌了，别停。"), 120);
    }

    const QRectF wallRect = chaseWallItem->sceneBoundingRect();
    bool p1Hit = p1Rect.intersects(wallRect);
    bool p2Hit = p2Rect.intersects(wallRect);
    if (!p1Hit && !p2Hit) {
        return;
    }

    if (p1Hit) {
        ++deathCount;
    }
    if (p2Hit) {
        ++deathCount;
    }

    if (p1Hit) {
        p1->setPos(lv.p1Spawn);
    }
    if (p2Hit) {
        p2->setPos(lv.p2Spawn);
    }
    chaseWallItem->setPos(0, 0);
    chaseWallIntroShown = false;

    if (p1Hit && p2Hit) {
        showDialogue(randomDialogue({
            QString::fromUtf8("里昂：相互靠着走，别倒在我前面。"),
            QString::fromUtf8("里昂：还没结束，别再丢下我一个人。"),
            QString::fromUtf8("里昂：你可不像是会轻易认输的人。")
        }) + "\n" + randomDialogue({
            QString::fromUtf8("艾达：清醒点，你的命还欠在我手上。"),
            QString::fromUtf8("艾达：别倒下，我可不想白来一趟。"),
            QString::fromUtf8("艾达：你最好不是故意的。")
        }), 120);
    } else if (p1Hit) {
        showDialogue(randomDialogue({
            QString::fromUtf8("艾达：清醒点，你的命还欠在我手上。"),
            QString::fromUtf8("艾达：别倒下，我可不想白来一趟。"),
            QString::fromUtf8("艾达：你最好不是故意的。")
        }), 120);
    } else {
        showDialogue(randomDialogue({
            QString::fromUtf8("里昂：相互靠着走，别倒在我前面。"),
            QString::fromUtf8("里昂：还没结束，别再丢下我一个人。"),
            QString::fromUtf8("里昂：你可不像是会轻易认输的人。")
        }), 120);
    }
}

void GameScene::checkIdleDialogue() {
    constexpr qreal moveEpsilon = 0.2;
    constexpr int idleTriggerFrames = 300;

    if (QLineF(p1->pos(), p1LastPos).length() <= moveEpsilon) {
        ++p1IdleFrames;
    } else {
        p1IdleFrames = 0;
    }

    if (QLineF(p2->pos(), p2LastPos).length() <= moveEpsilon) {
        ++p2IdleFrames;
    } else {
        p2IdleFrames = 0;
    }

    p1LastPos = p1->pos();
    p2LastPos = p2->pos();

    if (!p1IdleHintUsed && p1IdleFrames >= idleTriggerFrames) {
        p1IdleHintUsed = true;
        showDialogue(QString::fromUtf8("艾达：里昂，等什么呢？"), 100);
    }
    if (!p2IdleHintUsed && p2IdleFrames >= idleTriggerFrames) {
        p2IdleHintUsed = true;
        showDialogue(QString::fromUtf8("里昂：艾达？还好吗？"), 100);
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
        const qreal boxWidth = qBound<qreal>(360, viewRect.width() * 0.72, 620);
        const qreal boxHeight = 72;
        const qreal boxX = cx - boxWidth * 0.5;
        const qreal boxY = cy + halfH - 98;
        dialogueBoxItem->setRect(0, 0, boxWidth, boxHeight);
        dialogueBoxItem->setPos(boxX, boxY);
        dialogueTextItem->setTextWidth(boxWidth - 32);
        dialogueTextItem->setPos(boxX + 16, boxY + 11);
    }
}
