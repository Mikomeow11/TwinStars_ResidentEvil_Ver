#include "MyWindow.h"

#include "GameScene.h"
#include <QGraphicsView>

MyWindow::MyWindow(QWidget *parent) : QWidget(parent) {
    GameScene *scene = new GameScene(this);
    QGraphicsView *view = new QGraphicsView(scene, this);

    view->setFixedSize(800, 600);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setFocusPolicy(Qt::StrongFocus);

    view->setParent(this);
    setFixedSize(800, 600);
    view->setFocus();
}

MyWindow::~MyWindow() {}
