#include "ServerWindow.h"
#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QIcon icon(":icon.png");
    ServerWindow window;
    window.setWindowIcon(icon);

    QSystemTrayIcon *trayIcon = new QSystemTrayIcon(&window);
    trayIcon->setIcon(icon);

    QObject::connect(trayIcon, &QSystemTrayIcon::activated,
                     [&] (QSystemTrayIcon::ActivationReason reason)
    {
        switch (reason)
        {
            case QSystemTrayIcon::Trigger:
            case QSystemTrayIcon::DoubleClick:
                window.showNormal();
                window.activateWindow();
                break;
            default:
                break;
        }
    });

    trayIcon->show();
    window.show();
    return a.exec();
}
