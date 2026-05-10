#include "MyWindow.h"

#include "GameScene.h"
#include <QDir>
#include <QFont>
#include <QGraphicsView>
#include <QLabel>
#include <QPixmap>
#include <QStringList>
#include <QVBoxLayout>

MyWindow::MyWindow(QWidget *parent)
    : QWidget(parent),
      stacked(new QStackedWidget(this)),
      gameView(nullptr),
      endingTransitionPage(nullptr),
      endingCreditsPage(nullptr),
      endingCreditsContent(nullptr),
      endingScrollTimer(new QTimer(this)) {
    setFixedSize(800, 600);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->addWidget(stacked);

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

    QWidget* coverPage = createCoverPage();
    QWidget* prologuePage = createProloguePage();
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
    stacked->addWidget(storyGuidePage);
    stacked->addWidget(p1GuidePage);
    stacked->addWidget(p2GuidePage);

    QPushButton* startBtn = coverPage->findChild<QPushButton*>("startButton");
    QPushButton* prologueBtn = coverPage->findChild<QPushButton*>("prologueButton");
    QPushButton* backBtn = prologuePage->findChild<QPushButton*>("backButton");
    QPushButton* enterGameBtn = prologuePage->findChild<QPushButton*>("enterGameButton");
    QPushButton* storyNextBtn = storyGuidePage->findChild<QPushButton*>("storyNextButton");
    QPushButton* p1GuideNextBtn = p1GuidePage->findChild<QPushButton*>("p1GuideNextButton");
    QPushButton* p2GuideNextBtn = p2GuidePage->findChild<QPushButton*>("p2GuideNextButton");

    connect(startBtn, &QPushButton::clicked, this, [this]() { stacked->setCurrentIndex(2); });
    connect(prologueBtn, &QPushButton::clicked, this, [this]() { stacked->setCurrentIndex(1); });
    connect(backBtn, &QPushButton::clicked, this, [this]() { stacked->setCurrentIndex(0); });
    connect(enterGameBtn, &QPushButton::clicked, this, [this]() { stacked->setCurrentIndex(2); });
    connect(storyNextBtn, &QPushButton::clicked, this, [this]() { stacked->setCurrentIndex(3); });
    connect(p1GuideNextBtn, &QPushButton::clicked, this, [this]() { stacked->setCurrentIndex(4); });
    connect(p2GuideNextBtn, &QPushButton::clicked, this, &MyWindow::switchToGameView);
}

QWidget* MyWindow::createCoverPage() {
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
    auto *page = new QWidget();
    page->setFixedSize(800, 600);
    page->setStyleSheet("background-color: #140817;");

    auto *title = new QLabel(QString::fromUtf8("引言"), page);
    title->setGeometry(0, 54, 800, 56);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color: #fff4b8; background: transparent;");
    title->setFont(QFont(QStringLiteral("Microsoft YaHei"), 28, QFont::Bold));

    auto *placeholder = new QLabel(QString::fromUtf8("这里预留引言内容。\n后续可以改成文字排版，也可以直接替换为整张引言图片。"), page);
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

QPushButton* MyWindow::createTransparentButton(QWidget* parent, const QRect& geometry, const QString& objectName) {
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
    if (!gameView) {
        GameScene *scene = new GameScene(this);
        connect(scene, &GameScene::gameCompleted, this, &MyWindow::showEndingTransition);
        gameView = new QGraphicsView(scene);
        gameView->setFixedSize(800, 600);
        gameView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        gameView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        gameView->setFocusPolicy(Qt::StrongFocus);
        stacked->addWidget(gameView);
    }

    stacked->setCurrentWidget(gameView);
    gameView->setFocus();
}

void MyWindow::showEndingTransition(int herbsCollected, int totalHerbs, bool bearKeychainCollected, int deathCount) {
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
    auto *page = new QWidget();
    page->setFixedSize(800, 600);
    page->setStyleSheet("background-color:#08040c;");

    auto *text = new QLabel(page);
    text->setGeometry(80, 120, 640, 290);
    text->setAlignment(Qt::AlignCenter);
    text->setWordWrap(true);
    text->setStyleSheet("color:#fff4b8; background:transparent; font-size:26px; font-weight:700;");
    if (bearKeychainCollected) {
        text->setText(QString::fromUtf8("实验室的警报声渐渐远去。"));
    } else {
        text->setText(QString::fromUtf8("实验室的警报声渐渐远去。\n\n里昂：我们出来了。\n艾达：别回头，故事还没结束。"));
    }

    auto *nextButton = new QPushButton(QString::fromUtf8("查看结算"), page);
    nextButton->setGeometry(300, 462, 200, 54);
    nextButton->setStyleSheet(
        "QPushButton { background:#49dada; color:#211226; border:none; border-radius:8px; font-size:22px; font-weight:700; }"
        "QPushButton:hover { background:#75eeee; }"
    );
    connect(nextButton, &QPushButton::clicked, this, [this, herbsCollected, totalHerbs, bearKeychainCollected, deathCount]() {
        Q_UNUSED(herbsCollected);
        Q_UNUSED(totalHerbs);
        Q_UNUSED(bearKeychainCollected);
        Q_UNUSED(deathCount);
        stacked->setCurrentWidget(endingCreditsPage);
        if (!endingCreditsContent) {
            return;
        }
        endingCreditsContent->move(0, 600);
        endingScrollTimer->stop();
        endingScrollTimer->start(30);
    });

    return page;
}


QWidget* MyWindow::createEndingCreditsPage(int herbsCollected, int totalHerbs, bool bearKeychainCollected, int deathCount) {
    auto *page = new QWidget();
    page->setFixedSize(800, 600);
    page->setStyleSheet("background-color:#08040c;");

    QString achievements;
    if (herbsCollected == totalHerbs && totalHerbs > 0) {
        achievements += QString::fromUtf8("\n[生存专家] 收集所有草药补给");
    }
    if (bearKeychainCollected) {
        achievements += QString::fromUtf8("\n[旧日信物] 找到小熊钥匙扣");
    }
    if (herbsCollected == totalHerbs && totalHerbs > 0 && bearKeychainCollected) {
        achievements += QString::fromUtf8("\n[完美撤离] 带走所有补给与信物");
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

