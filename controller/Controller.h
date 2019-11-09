#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include "ClientSocket.h"

class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(ClientSocket *socket) noexcept;

    void startSending(int msInterval) noexcept;
    void stopSending() noexcept;

signals:
    void errorRaised(const QString &msg);
    void sent(int bytes);

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    ClientSocket *socket;
    int timerId;
};

#endif // CONTROLLER_H
