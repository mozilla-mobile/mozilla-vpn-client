#ifndef SIGNALHANDLER_H
#define SIGNALHANDLER_H

#include <QObject>

class SignalHandler : public QObject
{
    Q_OBJECT

public:
    SignalHandler();

private:
    static void saHandler(int signal);

signals:
    void quitRequested();
};

#endif // SIGNALHANDLER_H
