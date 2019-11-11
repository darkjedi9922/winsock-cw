#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QPlainTextEdit>

class Logger : public QObject
{
    Q_OBJECT

public:
    Logger(QPlainTextEdit *logEditor);

public slots:
    void write(const QString &message);
    void write(const QStringList &messages);

private:
    QPlainTextEdit *editor;
};

#endif // LOGGER_H
