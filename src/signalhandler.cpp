#include "signalhandler.h"
#include "signal.h"

#include <QDebug>

static SignalHandler *self = nullptr;

SignalHandler::SignalHandler()
{
    Q_ASSERT(!self);
    self = this;

    int quitSignals[] = {SIGQUIT, SIGINT, SIGTERM, SIGHUP};

    sigset_t mask;
    sigemptyset(&mask);
    for (auto sig : quitSignals) {
        sigaddset(&mask, sig);
    }

    struct sigaction sa;
    sa.sa_handler = SignalHandler::saHandler;
    sa.sa_mask = mask;
    sa.sa_flags = 0;

    for (auto sig : quitSignals) {
        sigaction(sig, &sa, nullptr);
    }
}

void SignalHandler::saHandler(int signal)
{
    qDebug() << "Signal" << signal;
    Q_ASSERT(self);
    emit self->quitRequested();
}
