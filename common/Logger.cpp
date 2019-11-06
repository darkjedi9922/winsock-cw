#include "Logger.h"

Logger::Logger(QPlainTextEdit *logEditor) : editor(logEditor) {}

void Logger::write(const QString &string)
{
    write(QStringList(string));
}

void Logger::write(const QStringList &messages)
{
    for (auto message : messages) {
        editor->appendPlainText(message);
    }
}
