#ifndef LOGGER_H
#define LOGGER_H

#include <QPlainTextEdit>

class Logger
{
public:
    Logger(QPlainTextEdit *logEditor);

    void write(const QString &message);
    void write(const QStringList &messages);

private:
    QPlainTextEdit *editor;
};

#endif // LOGGER_H
