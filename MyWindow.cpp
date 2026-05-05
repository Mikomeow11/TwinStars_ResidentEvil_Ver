#include "MyWindow.h"

#include "GameScene.h"
#include <QFont>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QPixmap>
#include <QVBoxLayout>

MyWindow::MyWindow(QWidget *parent) : QWidget(parent), stacked(new QStackedWidget(this)), gameView(nullptr) {
    setFixedSize(800, 600);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->addWidget(stacked);

    // Page 1: start menu
    QWidget* page1 = createIntroPage(
        ":/assets/ui/start_page.png",
        QString::fromUtf8("闪翼双星"),
        true
    );

    // Page 2: story
    QWidget* page2 = createIntroPage(
        ":/assets/ui/story_page.png",
        QString::fromUtf8("背景介绍页（你可替换背景图）"),
        false
    );

    // Page 3: tutorial part 1
    QWidget* page3 = createIntroPage(
        ":/assets/ui/tutorial_1.png",
        QString::fromUtf8("玩法介绍 1（你可替换背景图）"),
        false
    );

    // Page 4: tutorial part 2
    QWidget* page4 = createIntroPage(
        ":/assets/ui/tutorial_2.png",
        QString::fromUtf8("玩法介绍 2（点击箭头进入游戏）"),
        false
    );

    stacked->addWidget(page1);
    stacked->addWidget(page2);
    stacked->addWidget(page3);
    stacked->addWidget(page4);

    // Page transitions
    QPushButton* startBtn = page1->findChild<QPushButton*>("startButton");
    connect(startBtn, &QPushButton::clicked, this, [this]() { stacked->setCurrentIndex(1); });

    QPushButton* arrow2 = page2->findChild<QPushButton*>("arrowButton");
    QPushButton* arrow3 = page3->findChild<QPushButton*>("arrowButton");
    QPushButton* arrow4 = page4->findChild<QPushButton*>("arrowButton");

    connect(arrow2, &QPushButton::clicked, this, [this]() { stacked->setCurrentIndex(2); });
    connect(arrow3, &QPushButton::clicked, this, [this]() { stacked->setCurrentIndex(3); });
    connect(arrow4, &QPushButton::clicked, this, &MyWindow::switchToGameView);
}

QWidget* MyWindow::createIntroPage(const QString& backgroundRes, const QString& text, bool showStartButton) {
    auto *page = new QWidget();
    page->setFixedSize(800, 600);

    auto *bgLabel = new QLabel(page);
    bgLabel->setGeometry(0, 0, 800, 600);
    QPixmap bg(backgroundRes);
    if (!bg.isNull()) {
        bgLabel->setPixmap(bg.scaled(800, 600));
    } else {
        bgLabel->setStyleSheet("background-color: #2d114f;");
    }

    auto *title = new QLabel(text, page);
    title->setGeometry(40, 40, 720, 100);
    title->setStyleSheet("color: #d7ff9a; background: transparent;");
    title->setWordWrap(true);
    title->setAlignment(Qt::AlignCenter);
    QFont titleFont(QStringLiteral("Microsoft YaHei"), 22, QFont::Bold);
    title->setFont(titleFont);

    if (showStartButton) {
        auto *startButton = new QPushButton(QString::fromUtf8("开始游戏"), page);
        startButton->setObjectName("startButton");
        startButton->setGeometry(290, 470, 220, 64);
        startButton->setStyleSheet(
            "QPushButton { background:#57e3e3; color:#3b2a00; border-radius:18px; font-size:28px; font-weight:700; }"
            "QPushButton:hover { background:#7ef0f0; }"
        );
    } else {
        QPushButton* arrow = createArrowButton(page);
        arrow->setObjectName("arrowButton");
    }

    return page;
}

QPushButton* MyWindow::createArrowButton(QWidget* parent) {
    auto *arrow = new QPushButton(QString::fromUtf8("➜"), parent);
    arrow->setGeometry(700, 500, 72, 72);
    arrow->setStyleSheet(
        "QPushButton { background:#ffb300; color:white; border-radius:18px; font-size:36px; font-weight:700; }"
        "QPushButton:hover { background:#ffc933; }"
    );
    return arrow;
}

void MyWindow::switchToGameView() {
    if (!gameView) {
        GameScene *scene = new GameScene(this);
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

MyWindow::~MyWindow() {}
