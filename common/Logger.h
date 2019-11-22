#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QPlainTextEdit>
#include <fstream>

class Logger : public QObject
{
    Q_OBJECT

public:
    Logger(QPlainTextEdit *logEditor);
    ~Logger();

public slots:
    void write(const QString &message);
    void write(const QStringList &messages);

private:
    QPlainTextEdit *editor;
    std::ofstream file;
};

#endif // LOGGER_H
