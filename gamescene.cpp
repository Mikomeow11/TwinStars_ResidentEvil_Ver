#include "GameScene.h"

#include "Ground.h"
#include "Platform.h"
#include <QGraphicsView>
#include <QKeyEvent>
#include <QPen>

GameScene::GameScene(QObject *parent)
    : QGraphicsScene(parent),
      p1(new Player(1, Qt::cyan)),
      p2(new Player(2, Qt::magenta)),
      gameTimer(new QTimer(this)),
      currentLevelIndex(0),
      goalItem(nullptr),
      dialogueTextItem(new QGraphicsTextItem()),
      dialogueFramesLeft(0)
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
        { QRectF(240, 0, 120, 600), QString("Watch your jump timing"), false },
        { QRectF(1120, 0, 120, 600), QString("Almost at the goal"), false }
    };
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
    dialogueTextItem->setVisible(false);

    for (QGraphicsItem* item : levelItems) {
        removeItem(item);
        delete item;
    }
    levelItems.clear();
    goalItem = nullptr;

    setSceneRect(0, 0, lv.mapSize.width(), lv.mapSize.height());

    for (const RectBlock &g : lv.grounds) {
        auto *ground = new Ground(g.rect.x(), g.rect.y(), g.rect.width(), g.rect.height());
        levelItems.push_back(ground);
        addItem(ground);
    }

    for (const RectBlock &p : lv.platforms) {
        auto *platform = new Platform(p.rect.x(), p.rect.y(), p.rect.width(), p.rect.height());
        levelItems.push_back(platform);
        addItem(platform);
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
    updateCamera();

    if (dialogueFramesLeft > 0) {
        --dialogueFramesLeft;
        if (dialogueFramesLeft == 0) {
            dialogueTextItem->setVisible(false);
        }
    }
}

void GameScene::checkGoalAndMaybeSwitchLevel() {
    if (!goalItem) {
        return;
    }

    if (p1->sceneBoundingRect().intersects(goalItem->sceneBoundingRect())) {
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

void GameScene::updateCamera() {
    if (views().isEmpty()) {
        return;
    }

    QGraphicsView *view = views().first();
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
