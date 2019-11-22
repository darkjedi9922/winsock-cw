#include "ControllerWindow.h"
#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QIcon icon(":icon.png");
    ControllerWindow window;
    window.setWindowIcon(icon);
    window.show();
    return a.exec();
}
