/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dummypingsendworker.h"
#include "logger.h"

namespace {
Logger logger(LOG_NETWORKING, "DummyPingSendWorker");
}

void DummyPingSendWorker::sendPing(const QString &destination)
{
    logger.log() << "Dummy ping to:" << destination;
    emit pingSucceeded();
}
