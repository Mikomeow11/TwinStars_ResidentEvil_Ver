#include "MyWindow.h"

#include "GameScene.h"
#include <QAbstractAnimation>
#include <QDir>
#include <QFont>
#include <QGraphicsOpacityEffect>
#include <QGraphicsView>
#include <QLabel>
#include <QPixmap>
#include <QPair>
#include <QPropertyAnimation>
#include <QStringList>
#include <QVBoxLayout>

MyWindow::MyWindow(QWidget *parent)
    : QWidget(parent),
      stacked(new QStackedWidget(this)),
      gameView(nullptr),
      gameScene(nullptr),
      pausePage(nullptr),
      endingTransitionPage(nullptr),
      endingCreditsPage(nullptr),
      endingCreditsContent(nullptr),
      endingScrollTimer(new QTimer(this)) {
    setFixedSize(800, 600);

    // MyWindow 是整个游戏的“页面总控”，用 QStackedWidget 管理封面、引言、教程、游戏、暂停、结尾剧情和结算页。
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->addWidget(stacked);

    // 结算页滚屏计时器：通关后让统计信息和 guest 贺图从下往上滚动展示。
    connect(endingScrollTimer, &QTimer::timeout, this, [this]() {
        if (!endingCreditsContent) {
            endingScrollTimer->stop();
            return;
        }
        if (endingCreditsContent->y() > -endingCreditsContent->height()) {
            endingCreditsContent->move(endingCreditsContent->x(), endingCreditsContent->y() - 2);
        } else {
            endingScrollTimer->stop();
        }
    });

    // 按照实际游玩流程创建页面：封面 -> 引言/剧情 -> P1 键位 -> P2 键位 -> 游戏。
    QWidget* coverPage = createCoverPage();
    QWidget* prologuePage = createProloguePage();
    pausePage = createPausePage();
    QWidget* storyGuidePage = createGuidePage(
        ":/assets/ui/story_page.png",
        QString::fromUtf8("游戏背景剧情页\n图片完成后请放到 assets/ui/story_page.png"),
        "storyNextButton"
    );
    QWidget* p1GuidePage = createGuidePage(
        ":/assets/ui/tutorial_p1.png",
        QString::fromUtf8("P1 键位介绍\nW / A / S / D 控制移动"),
        "p1GuideNextButton"
    );
    QWidget* p2GuidePage = createGuidePage(
        ":/assets/ui/tutorial_p2.png",
        QString::fromUtf8("P2 键位介绍\n方向键控制移动"),
        "p2GuideNextButton"
    );

    stacked->addWidget(coverPage);
    stacked->addWidget(prologuePage);
    stacked->addWidget(pausePage);
    stacked->addWidget(storyGuidePage);
    stacked->addWidget(p1GuidePage);
    stacked->addWidget(p2GuidePage);

    // 这里把每个按钮和页面跳转连接起来，录屏时可以点着讲“页面切换流程都集中在 MyWindow.cpp”。
    QPushButton* startBtn = coverPage->findChild<QPushButton*>("startButton");
    QPushButton* prologueBtn = coverPage->findChild<QPushButton*>("prologueButton");
    QPushButton* backBtn = prologuePage->findChild<QPushButton*>("backButton");
    QPushButton* enterGameBtn = prologuePage->findChild<QPushButton*>("enterGameButton");
    QPushButton* storyNextBtn = storyGuidePage->findChild<QPushButton*>("storyNextButton");
    QPushButton* p1GuideNextBtn = p1GuidePage->findChild<QPushButton*>("p1GuideNextButton");
    QPushButton* p2GuideNextBtn = p2GuidePage->findChild<QPushButton*>("p2GuideNextButton");

    connect(startBtn, &QPushButton::clicked, this, [this]() { stacked->setCurrentIndex(3); });
    connect(prologueBtn, &QPushButton::clicked, this, [this]() { stacked->setCurrentIndex(1); });
    connect(backBtn, &QPushButton::clicked, this, [this]() { stacked->setCurrentIndex(0); });
    connect(enterGameBtn, &QPushButton::clicked, this, [this]() { stacked->setCurrentIndex(3); });
    connect(storyNextBtn, &QPushButton::clicked, this, [this]() { stacked->setCurrentIndex(4); });
    connect(p1GuideNextBtn, &QPushButton::clicked, this, [this]() { stacked->setCurrentIndex(5); });
    connect(p2GuideNextBtn, &QPushButton::clicked, this, &MyWindow::switchToGameView);
}

QWidget* MyWindow::createCoverPage() {
    // 封面页：背景图来自 resources.qrc，按钮做成透明区域，位置配合自己设计的封面图。
    auto *page = new QWidget();
    page->setFixedSize(800, 600);

    auto *bgLabel = new QLabel(page);
    bgLabel->setGeometry(0, 0, 800, 600);
    QPixmap bg(":/assets/ui/start_page.png");
    if (!bg.isNull()) {
        bgLabel->setPixmap(bg.scaled(800, 600, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    } else {
        bgLabel->setStyleSheet("background-color: #2d114f;");
    }

    createTransparentButton(page, QRect(386, 442, 352, 46), "startButton");
    createTransparentButton(page, QRect(364, 503, 354, 45), "prologueButton");

    return page;
}

QWidget* MyWindow::createProloguePage() {
    // 引言页：放作业说明、同人声明和图片来源说明，避免直接进入游戏太突兀。
    auto *page = new QWidget();
    page->setFixedSize(800, 600);
    page->setStyleSheet("background-color: #140817;");

    auto *title = new QLabel(QString::fromUtf8("引言"), page);
    title->setGeometry(0, 54, 800, 56);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color: #fff4b8; background: transparent;");
    title->setFont(QFont(QStringLiteral("Microsoft YaHei"), 28, QFont::Bold));

    auto *placeholder = new QLabel(QString::fromUtf8(
        "1. 粗糙小游戏博大家一笑，请勿较真；\n"
        "2. 【同人向】生化危机9衍生作品，含 aeon（里昂艾达）；\n"
        "3. 部分图源网络，仅作学习参考使用。"
    ), page);
    placeholder->setGeometry(110, 170, 580, 190);
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setWordWrap(true);
    placeholder->setStyleSheet(
        "color: white;"
        "background: rgba(255, 255, 255, 24);"
        "border: 2px solid rgba(255, 244, 184, 150);"
        "border-radius: 8px;"
        "font-size: 22px;"
        "line-height: 1.5;"
    );

    auto *backButton = new QPushButton(QString::fromUtf8("返回"), page);
    backButton->setObjectName("backButton");
    backButton->setGeometry(190, 464, 170, 52);
    backButton->setStyleSheet(
        "QPushButton { background:#3d203e; color:#fff4b8; border:2px solid #fff4b8; border-radius:8px; font-size:22px; font-weight:700; }"
        "QPushButton:hover { background:#57305a; }"
    );

    auto *enterGameButton = new QPushButton(QString::fromUtf8("开始游戏"), page);
    enterGameButton->setObjectName("enterGameButton");
    enterGameButton->setGeometry(440, 464, 170, 52);
    enterGameButton->setStyleSheet(
        "QPushButton { background:#49dada; color:#311b35; border:none; border-radius:8px; font-size:22px; font-weight:700; }"
        "QPushButton:hover { background:#71eeee; }"
    );

    return page;
}

QWidget* MyWindow::createGuidePage(const QString& imageRes, const QString& fallbackText, const QString& buttonName) {
    // 通用教程页：剧情图、P1 键位图、P2 键位图都复用这个函数，减少重复代码。
    auto *page = new QWidget();
    page->setFixedSize(800, 600);

    auto *bgLabel = new QLabel(page);
    bgLabel->setGeometry(0, 0, 800, 600);
    QPixmap bg(imageRes);
    if (!bg.isNull()) {
        bgLabel->setPixmap(bg.scaled(800, 600, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    } else {
        bgLabel->setStyleSheet("background-color: #210832;");

        auto *placeholder = new QLabel(fallbackText, page);
        placeholder->setGeometry(86, 110, 628, 300);
        placeholder->setAlignment(Qt::AlignCenter);
        placeholder->setWordWrap(true);
        placeholder->setStyleSheet(
            "color: #d7ff9a;"
            "background: rgba(0, 0, 0, 70);"
            "border: 2px solid rgba(215, 255, 154, 160);"
            "border-radius: 8px;"
            "font-size: 28px;"
            "font-weight: 700;"
            "line-height: 1.5;"
        );
    }

    createNextButton(page, buttonName);
    return page;
}

QWidget* MyWindow::createPausePage() {
    // 暂停菜单：GameScene 中按 Esc 会发出 pauseRequested 信号，然后切到这个页面。
    auto *page = new QWidget();
    page->setFixedSize(800, 600);
    page->setStyleSheet("background-color:#08040c;");

    auto *title = new QLabel(QString::fromUtf8("暂停"), page);
    title->setGeometry(0, 105, 800, 64);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color:#fff4b8; background:transparent; font-size:42px; font-weight:800;");

    auto makeButton = [page](const QString &text, const QString &name, int y) {
        auto *button = new QPushButton(text, page);
        button->setObjectName(name);
        button->setGeometry(300, y, 200, 54);
        button->setCursor(Qt::PointingHandCursor);
        button->setFocusPolicy(Qt::NoFocus);
        button->setStyleSheet(
            "QPushButton { background:#49dada; color:#211226; border:none; border-radius:8px; font-size:22px; font-weight:700; }"
            "QPushButton:hover { background:#75eeee; }"
            "QPushButton:pressed { background:#2abcbc; }"
        );
        return button;
    };

    auto *resumeButton = makeButton(QString::fromUtf8("继续游戏"), "resumeButton", 220);
    auto *restartButton = makeButton(QString::fromUtf8("重开本关"), "restartLevelButton", 294);
    auto *coverButton = makeButton(QString::fromUtf8("返回封面"), "returnCoverButton", 368);

    // 继续游戏：切回 QGraphicsView，并恢复 GameScene 的计时器。
    connect(resumeButton, &QPushButton::clicked, this, [this]() {
        if (!gameView || !gameScene) {
            return;
        }
        stacked->setCurrentWidget(gameView);
        gameScene->resumeGame();
        gameView->setFocus();
    });

    // 重开本关：调用 GameScene::restartCurrentLevel，只重置当前关卡，不清空整体收集统计。
    connect(restartButton, &QPushButton::clicked, this, [this]() {
        if (!gameView || !gameScene) {
            return;
        }
        gameScene->restartCurrentLevel();
        stacked->setCurrentWidget(gameView);
        gameView->setFocus();
    });

    // 返回封面：销毁当前游戏视图和结尾页面，下次开始会重新创建一局。
    connect(coverButton, &QPushButton::clicked, this, [this]() {
        if (endingScrollTimer->isActive()) {
            endingScrollTimer->stop();
        }
        if (gameScene) {
            gameScene->stopGame();
        }
        if (gameView) {
            stacked->removeWidget(gameView);
            gameView->deleteLater();
            gameView = nullptr;
            gameScene = nullptr;
        }
        if (endingTransitionPage) {
            stacked->removeWidget(endingTransitionPage);
            endingTransitionPage->deleteLater();
            endingTransitionPage = nullptr;
        }
        if (endingCreditsPage) {
            stacked->removeWidget(endingCreditsPage);
            endingCreditsPage->deleteLater();
            endingCreditsPage = nullptr;
            endingCreditsContent = nullptr;
        }
        stacked->setCurrentIndex(0);
    });

    return page;
}

QPushButton* MyWindow::createTransparentButton(QWidget* parent, const QRect& geometry, const QString& objectName) {
    // 透明按钮：视觉上只看到封面图，鼠标点击区域由代码中的 QRect 精确控制。
    auto *button = new QPushButton(parent);
    button->setObjectName(objectName);
    button->setGeometry(geometry);
    button->setCursor(Qt::PointingHandCursor);
    button->setFocusPolicy(Qt::NoFocus);
    button->setStyleSheet(
        "QPushButton { background: transparent; border: none; }"
        "QPushButton:hover { background: rgba(255, 255, 255, 28); border-radius: 8px; }"
        "QPushButton:pressed { background: rgba(0, 0, 0, 28); border-radius: 8px; }"
    );
    return button;
}

QPushButton* MyWindow::createNextButton(QWidget* parent, const QString& objectName) {
    // 下一页按钮：教程页共用，保证剧情页和键位说明页的交互一致。
    auto *button = new QPushButton(QString::fromUtf8("➜"), parent);
    button->setObjectName(objectName);
    button->setGeometry(686, 480, 78, 78);
    button->setCursor(Qt::PointingHandCursor);
    button->setFocusPolicy(Qt::NoFocus);
    button->setStyleSheet(
        "QPushButton { background:#ffb21c; color:white; border:none; border-radius:8px; font-size:42px; font-weight:900; }"
        "QPushButton:hover { background:#ffc84a; }"
        "QPushButton:pressed { background:#e69c12; }"
    );
    return button;
}

void MyWindow::switchToGameView() {
    // 第一次进入游戏时才创建 GameScene 和 QGraphicsView，避免封面阶段就启动游戏循环。
    if (!gameView) {
        gameScene = new GameScene(this);
        connect(gameScene, &GameScene::gameCompleted, this, &MyWindow::showEndingTransition);
        connect(gameScene, &GameScene::pauseRequested, this, [this]() {
            if (pausePage) {
                stacked->setCurrentWidget(pausePage);
            }
        });
        gameView = new QGraphicsView(gameScene);
        gameView->setFixedSize(800, 600);
        gameView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        gameView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        gameView->setFocusPolicy(Qt::StrongFocus);
        stacked->addWidget(gameView);
    }

    stacked->setCurrentWidget(gameView);
    if (gameScene) {
        gameScene->resumeGame();
    }
    gameView->setFocus();
}

void MyWindow::showEndingTransition(int herbsCollected, int totalHerbs, bool bearKeychainCollected, int deathCount) {
    // 通关后由 GameScene 发出 gameCompleted 信号，MyWindow 负责切到结尾剧情和结算页面。
    if (endingScrollTimer->isActive()) {
        endingScrollTimer->stop();
    }
    if (endingTransitionPage) {
        stacked->removeWidget(endingTransitionPage);
        endingTransitionPage->deleteLater();
        endingTransitionPage = nullptr;
    }
    if (endingCreditsPage) {
        stacked->removeWidget(endingCreditsPage);
        endingCreditsPage->deleteLater();
        endingCreditsPage = nullptr;
        endingCreditsContent = nullptr;
    }

    endingTransitionPage = createEndingTransitionPage(herbsCollected, totalHerbs, bearKeychainCollected, deathCount);
    endingCreditsPage = createEndingCreditsPage(herbsCollected, totalHerbs, bearKeychainCollected, deathCount);
    stacked->addWidget(endingTransitionPage);
    stacked->addWidget(endingCreditsPage);
    stacked->setCurrentWidget(endingTransitionPage);
}

QWidget* MyWindow::createEndingTransitionPage(int herbsCollected, int totalHerbs, bool bearKeychainCollected, int deathCount) {
    // 结尾剧情页：做成类似文字游戏的对话框，点击对白框或“下一句”推进剧情。
    Q_UNUSED(herbsCollected);
    Q_UNUSED(totalHerbs);
    Q_UNUSED(bearKeychainCollected);

    auto *page = new QWidget();
    page->setFixedSize(800, 600);
    page->setStyleSheet("background-color:#08040c;");

    auto *bgLabel = new QLabel(page);
    bgLabel->setGeometry(0, 0, 800, 600);
    bgLabel->setStyleSheet("background-color:#08040c;");
    QPixmap endingBg(":/assets/ui/ending_transition.png");
    if (!endingBg.isNull()) {
        bgLabel->setPixmap(endingBg.scaled(800, 600, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }

    auto *adaPortrait = new QLabel(page);
    adaPortrait->setGeometry(42, 120, 250, 375);
    adaPortrait->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
    adaPortrait->setStyleSheet("background:transparent;");
    QPixmap adaPixmap(":/assets/characters/p2_idle.png.png");
    if (!adaPixmap.isNull()) {
        adaPortrait->setPixmap(adaPixmap.scaled(250, 375, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    auto *leonPortrait = new QLabel(page);
    leonPortrait->setGeometry(508, 120, 250, 375);
    leonPortrait->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
    leonPortrait->setStyleSheet("background:transparent;");
    QPixmap leonPixmap(":/assets/characters/p1_idle.png.png");
    if (!leonPixmap.isNull()) {
        leonPortrait->setPixmap(leonPixmap.scaled(250, 375, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    using EndingLine = QPair<QString, QString>;
    // 死亡次数会影响结尾台词：死亡较多时语气更像两个人硬撑着逃出来。
    const QVector<EndingLine> endingLines = deathCount >= 8
        ? QVector<EndingLine>{
            { QString::fromUtf8("旁白"), QString::fromUtf8("实验室的警报声渐渐远去。") },
            { QString::fromUtf8("艾达"), QString::fromUtf8("呵……是出口。还能走吗，里昂？要一起吗？") },
            { QString::fromUtf8("里昂"), QString::fromUtf8("我哪有拒绝的余地……还是说，你不打算救人救到底？") },
            { QString::fromUtf8("艾达"), QString::fromUtf8("我向来不喜欢做一半就收手。") },
            { QString::fromUtf8("里昂"), QString::fromUtf8("只是单纯不想半途而废吗？") },
            { QString::fromUtf8("艾达"), QString::fromUtf8("你觉得我会随便浪费精力，跑来这种地方？") },
            { QString::fromUtf8("里昂"), QString::fromUtf8("所以……你是专程来救我的？") },
            { QString::fromUtf8("艾达"), QString::fromUtf8("明知故问。") },
            { QString::fromUtf8("里昂"), QString::fromUtf8("那我欠你的，看来不止一条命。") },
            { QString::fromUtf8("艾达"), QString::fromUtf8("记着就好。别再把它随便丢在这种地方。") }
        }
        : QVector<EndingLine>{
            { QString::fromUtf8("旁白"), QString::fromUtf8("实验室的警报声渐渐远去。") },
            { QString::fromUtf8("艾达"), QString::fromUtf8("呵……是出口。看起来，你还没狼狈到需要我扶。一起吗？") },
            { QString::fromUtf8("里昂"), QString::fromUtf8("我现在当然不会再拒绝了……不过，你真的打算救人救到底？") },
            { QString::fromUtf8("艾达"), QString::fromUtf8("我既然出现在这里，就没打算看着你死在这里。") },
            { QString::fromUtf8("里昂"), QString::fromUtf8("我还以为，你只对任务目标感兴趣。") },
            { QString::fromUtf8("艾达"), QString::fromUtf8("任务早就完成了。但我还没收到我想听的那句“谢谢”。") },
            { QString::fromUtf8("里昂"), QString::fromUtf8("所以，你这次是为了我？而不是别的什么？") },
            { QString::fromUtf8("艾达"), QString::fromUtf8("你总是喜欢把简单的问题问得很复杂。") },
            { QString::fromUtf8("里昂"), QString::fromUtf8("那我就简单一点。谢谢你，艾达。") },
            { QString::fromUtf8("艾达"), QString::fromUtf8("这句还算像样。") },
            { QString::fromUtf8("里昂"), QString::fromUtf8("我们接下来去哪？") },
            { QString::fromUtf8("艾达"), QString::fromUtf8("先离开这里。至于之后……看你表现。") }
        };

    auto *speakerLabel = new QLabel(page);
    speakerLabel->setGeometry(72, 388, 150, 40);
    speakerLabel->setAlignment(Qt::AlignCenter);
    speakerLabel->setStyleSheet(
        "color:#211226;"
        "background:#df9fb6;"
        "border-radius:8px;"
        "font-size:22px;"
        "font-weight:700;"
    );

    auto *dialogueBox = new QLabel(page);
    dialogueBox->setGeometry(48, 420, 704, 118);
    dialogueBox->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    dialogueBox->setWordWrap(true);
    dialogueBox->setStyleSheet(
        "color:#fff4b8;"
        "background:rgba(0,0,0,190);"
        "border:2px solid #df9fb6;"
        "border-radius:8px;"
        "padding:18px;"
        "font-size:22px;"
        "font-weight:700;"
        "line-height:1.35;"
    );

    auto *dialogueClickArea = new QPushButton(page);
    dialogueClickArea->setGeometry(dialogueBox->geometry());
    dialogueClickArea->setCursor(Qt::PointingHandCursor);
    dialogueClickArea->setFocusPolicy(Qt::NoFocus);
    dialogueClickArea->setStyleSheet("QPushButton { background:transparent; border:none; }");

    auto *nextButton = new QPushButton(page);
    nextButton->setGeometry(650, 548, 102, 38);
    nextButton->setProperty("dialogueIndex", 0);
    nextButton->setStyleSheet(
        "QPushButton { background:#49dada; color:#211226; border:none; border-radius:8px; font-size:18px; font-weight:700; }"
        "QPushButton:hover { background:#75eeee; }"
    );

    auto *adaOpacity = new QGraphicsOpacityEffect(adaPortrait);
    auto *leonOpacity = new QGraphicsOpacityEffect(leonPortrait);
    adaPortrait->setGraphicsEffect(adaOpacity);
    leonPortrait->setGraphicsEffect(leonOpacity);

    auto updateEndingLine = [adaOpacity, leonOpacity, speakerLabel, dialogueBox, nextButton, endingLines](int index) {
        // 根据当前说话人调整立绘透明度，让观众一眼看出是谁在讲话。
        if (endingLines.isEmpty()) {
            return;
        }
        const int safeIndex = qBound(0, index, endingLines.size() - 1);
        const QString speaker = endingLines.at(safeIndex).first;
        const QString line = endingLines.at(safeIndex).second;

        speakerLabel->setText(speaker);
        dialogueBox->setText(line);
        nextButton->setText(safeIndex == endingLines.size() - 1 ? QString::fromUtf8("查看结算") : QString::fromUtf8("下一句"));

        const bool adaSpeaking = speaker == QString::fromUtf8("艾达");
        const bool leonSpeaking = speaker == QString::fromUtf8("里昂");
        adaOpacity->setOpacity((adaSpeaking || !leonSpeaking) ? 1.0 : 0.42);
        leonOpacity->setOpacity((leonSpeaking || !adaSpeaking) ? 1.0 : 0.42);
    };
    updateEndingLine(0);

    auto advanceEndingDialogue = [this, nextButton, updateEndingLine, endingLines]() {
        // 剧情播放完后进入滚屏结算页。
        const int nextIndex = nextButton->property("dialogueIndex").toInt() + 1;
        if (nextIndex < endingLines.size()) {
            nextButton->setProperty("dialogueIndex", nextIndex);
            updateEndingLine(nextIndex);
            return;
        }

        stacked->setCurrentWidget(endingCreditsPage);
        if (!endingCreditsContent) {
            return;
        }
        endingCreditsContent->move(0, 600);
        endingScrollTimer->stop();
        endingScrollTimer->start(30);
    };
    connect(nextButton, &QPushButton::clicked, this, advanceEndingDialogue);
    connect(dialogueClickArea, &QPushButton::clicked, this, advanceEndingDialogue);

    auto *fadeOverlay = new QWidget(page);
    fadeOverlay->setGeometry(0, 0, 800, 600);
    fadeOverlay->setStyleSheet("background-color:black;");
    fadeOverlay->raise();
    auto *fadeEffect = new QGraphicsOpacityEffect(fadeOverlay);
    fadeOverlay->setGraphicsEffect(fadeEffect);
    fadeEffect->setOpacity(1.0);

    auto *fadeAnimation = new QPropertyAnimation(fadeEffect, "opacity", page);
    fadeAnimation->setDuration(1600);
    fadeAnimation->setStartValue(1.0);
    fadeAnimation->setEndValue(0.0);
    connect(fadeAnimation, &QPropertyAnimation::finished, fadeOverlay, &QWidget::hide);
    fadeAnimation->start(QAbstractAnimation::DeleteWhenStopped);

    return page;
}


QWidget* MyWindow::createEndingCreditsPage(int herbsCollected, int totalHerbs, bool bearKeychainCollected, int deathCount) {
    // 结算页：汇总草药、小熊钥匙扣、死亡次数，并根据条件解锁成就。
    auto *page = new QWidget();
    page->setFixedSize(800, 600);
    page->setStyleSheet("background-color:#08040c;");

    QString achievements;
    // 成就判断集中在这里，录屏时可以直接展示“生存专家 / 旧日信物 / 完美撤离 / 无伤撤离”。
    if (herbsCollected == totalHerbs && totalHerbs > 0) {
        achievements += QString::fromUtf8("\n[生存专家] 收集所有草药补给");
    }
    if (bearKeychainCollected) {
        achievements += QString::fromUtf8("\n[旧日信物] 找到小熊钥匙扣");
    }
    if (herbsCollected == totalHerbs && totalHerbs > 0 && bearKeychainCollected) {
        achievements += QString::fromUtf8("\n[完美撤离] 带走所有补给与信物");
    }
    if (deathCount == 0) {
        achievements += QString::fromUtf8("\n[无伤撤离] 全程零死亡通关");
    }
    if (achievements.isEmpty()) {
        achievements = QString::fromUtf8("\n暂无隐藏成就解锁");
    }

    auto *content = new QWidget(page);
    content->setObjectName("endingCreditsContent");
    content->setStyleSheet("background:transparent;");
    endingCreditsContent = content;

    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(70, 0, 70, 80);
    layout->setSpacing(28);

    auto *summaryLabel = new QLabel(content);
    summaryLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    summaryLabel->setWordWrap(true);
    summaryLabel->setStyleSheet("color:#d7ff9a; background:transparent; font-size:24px; font-weight:700; line-height:1.5;");
    summaryLabel->setText(QString::fromUtf8(
        "任务完成\n\n"
        "双星已撤离实验室\n\n"
        "草药收集：%1 / %2\n"
        "小熊钥匙扣：%3\n"
        "死亡次数：%4\n\n"
        "成就解锁：%5\n\n"
        "感谢游玩"
    ).arg(herbsCollected)
     .arg(totalHerbs)
     .arg(bearKeychainCollected ? QString::fromUtf8("已获得") : QString::fromUtf8("未获得"))
     .arg(deathCount)
     .arg(achievements));
    layout->addWidget(summaryLabel);

    // guest 贺图统一从资源目录读取，后续只要在 resources.qrc 里登记图片即可自动展示。
    QDir guestDir(":/assets/ui/guests");
    const QStringList guestFiles = guestDir.entryList({ "*.png", "*.jpg", "*.jpeg" }, QDir::Files, QDir::Name);
    if (!guestFiles.isEmpty()) {
        auto *guestTitle = new QLabel(QString::fromUtf8("贺图展示"), content);
        guestTitle->setAlignment(Qt::AlignCenter);
        guestTitle->setStyleSheet("color:#fff4b8; background:transparent; font-size:24px; font-weight:700;");
        layout->addWidget(guestTitle);

        for (const QString &fileName : guestFiles) {
            const QString resourcePath = QString(":/assets/ui/guests/%1").arg(fileName);
            QPixmap guestPixmap(resourcePath);
            if (guestPixmap.isNull()) {
                continue;
            }

            auto *guestImage = new QLabel(content);
            guestImage->setAlignment(Qt::AlignCenter);
            guestImage->setStyleSheet("background:transparent;");
            guestImage->setPixmap(guestPixmap.scaled(620, 420, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            layout->addWidget(guestImage);
        }
    }

    layout->activate();
    content->setGeometry(0, 600, 800, layout->sizeHint().height());

    return page;
}

MyWindow::~MyWindow() {}

