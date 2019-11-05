#include "QMainWindow.h"
#include "ui_QMainWindow.h"

QMainWindow::QMainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QMainWindow)
{
    ui->setupUi(this);
    ui->clientsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

QMainWindow::~QMainWindow()
{
    delete ui;
}