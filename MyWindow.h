#ifndef MYWINDOW_H
#define MYWINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class MyWindow;
}
QT_END_NAMESPACE

class MyWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MyWindow(QWidget *parent = nullptr);
    ~MyWindow() override;

private:
    Ui::MyWindow *ui;
};
#endif // MYWINDOW_H
