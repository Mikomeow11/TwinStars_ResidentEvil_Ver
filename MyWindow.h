#ifndef MYWINDOW_H
#define MYWINDOW_H

#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QWidget>

class QGraphicsView;

class MyWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MyWindow(QWidget *parent = nullptr);
    ~MyWindow() override;

private:
    QWidget* createIntroPage(const QString& backgroundRes, const QString& text, bool showStartButton);
    QPushButton* createArrowButton(QWidget* parent);
    void switchToGameView();

    QStackedWidget* stacked;
    QGraphicsView* gameView;
};
#endif // MYWINDOW_H
