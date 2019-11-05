#ifndef QMAINWINDOW_H
#define QMAINWINDOW_H

#include <QWidget>

namespace Ui {
class QMainWindow;
}

class QMainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit QMainWindow(QWidget *parent = nullptr);
    ~QMainWindow();

private:
    Ui::QMainWindow *ui;
};

#endif // QMAINWINDOW_H
