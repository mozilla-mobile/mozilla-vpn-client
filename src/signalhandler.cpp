/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "signalhandler.h"
#include "logger.h"
#include "signal.h"

namespace {

Logger logger("SignalHandler");
SignalHandler *self = nullptr;

} // namespace

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
    logger.log() << "Signal" << signal;
    Q_ASSERT(self);
    emit self->quitRequested();
}
