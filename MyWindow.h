#ifndef MYWINDOW_H
#define MYWINDOW_H

#include <QLabel>
#include <QPushButton>
#include <QRect>
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
    QWidget* createCoverPage();
    QWidget* createProloguePage();
    QWidget* createGuidePage(const QString& imageRes, const QString& fallbackText, const QString& buttonName);
    QPushButton* createTransparentButton(QWidget* parent, const QRect& geometry, const QString& objectName);
    QPushButton* createNextButton(QWidget* parent, const QString& objectName);
    void switchToGameView();

    QStackedWidget* stacked;
    QGraphicsView* gameView;
};
#endif // MYWINDOW_H
