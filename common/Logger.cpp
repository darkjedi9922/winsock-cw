#include "Logger.h"
#include <QDateTime>

Logger::Logger(QPlainTextEdit *logEditor) :
    QObject(),
    editor(logEditor)
{}

void Logger::write(const QString &string)
{
    write(QStringList(string));
}

void Logger::write(const QStringList &messages)
{
    QDateTime datetime = QDateTime::currentDateTime();

    for (auto message : messages) {
        QString timeMessage = "[" + datetime.toString("dd.MM.yyyy hh:mm:ss") + "] ";
        editor->appendPlainText(timeMessage + message);
    }
}
