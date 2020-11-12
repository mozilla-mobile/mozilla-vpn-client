/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "pingsender.h"
#include "logger.h"
#include "pingsendworker.h"

#if defined(MVPN_LINUX) || defined(MVPN_ANDROID)
#include "platforms/linux/linuxpingsendworker.h"
#elif defined(MVPN_MACOS) || defined(MVPN_IOS)
#include "platforms/macos/macospingsendworker.h"
#else
#include "platforms/dummy/dummypingsendworker.h"
#endif

#ifdef QT_DEBUG
#include "platforms/dummy/dummypingsendworker.h"
#endif

namespace {
Logger logger(LOG_NETWORKING, "PingSender");
}

PingSender::PingSender(QObject *parent) : QObject(parent)
{
    PingSendWorker *worker =
#if defined(MVPN_LINUX) || defined(MVPN_ANDROID)
        new LinuxPingSendWorker();
#elif defined(MVPN_MACOS) || defined(MVPN_IOS)
        new MacOSPingSendWorker();
#else
        new DummyPingSendWorker(DummyPingSendWorker::Stable);
#endif

    // Uncomment the following lines to enable the DummyPingSendWorker. There are 3 modes:
    // - Stable: the network is stable
    // - Unstable: after 5 seconds, we go to unstable
    // - NoSignal: after 30 seconds (after the Unstable state) we go to no-signal
    //
    //#ifdef QT_DEBUG
    //    worker = new DummyPingSendWorker(DummyPingSendWorker::NoSignal);
    //#endif

    worker->moveToThread(&m_workerThread);

    connect(&m_workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &PingSender::sendPing, worker, &PingSendWorker::sendPing);
    connect(this, &PingSender::stopPing, worker, &PingSendWorker::stopPing);
    connect(worker, &PingSendWorker::pingFailed, this, &PingSender::pingFailed);
    connect(worker, &PingSendWorker::pingSucceeded, this, &PingSender::pingSucceeded);

    m_workerThread.start();
}

PingSender::~PingSender()
{
    m_workerThread.quit();
    m_workerThread.wait();
}

void PingSender::send(const QString &destination)
{
    logger.log() << "PingSender send to" << destination;

    m_active = true;
    emit sendPing(destination);
}

void PingSender::stop()
{
    logger.log() << "PingSender stop";
    m_active = false;
    emit stopPing();
}

void PingSender::pingFailed()
{
    logger.log() << "PingSender - Ping Failed";

    if (m_active) {
        emit completed();
    }
}

void PingSender::pingSucceeded()
{
    logger.log() << "PingSender - Ping Succeded";
    if (m_active) {
        emit completed();
    }
}
