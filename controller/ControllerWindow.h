#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

namespace Ui {
class ControllerWindow;
}

class ControllerWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ControllerWindow(QWidget *parent = nullptr);
    ~ControllerWindow();

private:
    Ui::ControllerWindow *ui;
};

#endif // MAINWINDOW_H
