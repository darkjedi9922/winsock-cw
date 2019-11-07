#include "ControllerWindow.h"
#include "ui_ControllerWindow.h"

ControllerWindow::ControllerWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControllerWindow)
{
    ui->setupUi(this);
}

ControllerWindow::~ControllerWindow()
{
    delete ui;
}
