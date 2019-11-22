#include "Logger.h"
#include <QDateTime>
#include <QDir>

using namespace std;

Logger::Logger(QPlainTextEdit *logEditor) :
    QObject(),
    editor(logEditor)
{
    QDir dir;
    if (!dir.exists("logs")) dir.mkdir("logs");

    auto filename = string("logs/") + to_string(time(nullptr))
            + "-" + logEditor->whatsThis().toStdString() + ".txt";
    file.open(filename);

    if (!file.is_open())
        this->write(QString("Cannot open file %1").arg(filename.c_str()));
}

Logger::~Logger()
{
    if (file.is_open()) file.close();
}

void Logger::write(const QString &string)
{
    write(QStringList(string));
}

void Logger::write(const QStringList &messages)
{
    QDateTime datetime = QDateTime::currentDateTime();

    for (auto message : messages) {
        QString timeMessage = "[" + datetime.toString("dd.MM.yyyy hh:mm:ss") + "] ";
        QString output = timeMessage + message;
        editor->appendPlainText(output);
        file << output.toStdString().c_str() << endl;
    }
}
