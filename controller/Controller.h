#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include "ClientSocket.h"
#include "definitions.h"

class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(ClientSocket *socket) noexcept;

    short getNumber() const;
    void setNumber(short number) noexcept;

    void startSending(int msInterval) noexcept;
    void stopSending() noexcept;

signals:
    void errorRaised(const QString &msg);
    void sent(int bytes);

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    ClientSocket *socket;
    short number;
    int timerId;

    void generateAndSend() noexcept;
};

#endif // CONTROLLER_H
