/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "pingsender.h"
#include "leakdetector.h"
#include "logger.h"
#include "pingsendworker.h"

#if defined(MVPN_LINUX) || defined(MVPN_ANDROID)
#  include "platforms/linux/linuxpingsendworker.h"
#elif defined(MVPN_MACOS) || defined(MVPN_IOS)
#  include "platforms/macos/macospingsendworker.h"
#elif defined(MVPN_WINDOWS)
#  include "platforms/windows/windowspingsendworker.h"
#elif defined(MVPN_DUMMY) || defined(UNIT_TEST)
#  include "platforms/dummy/dummypingsendworker.h"
#else
#  error "Unsupported platform"
#endif

#include <QThread>

namespace {
Logger logger(LOG_NETWORKING, "PingSender");
}

PingSender::PingSender(QObject* parent, QThread* thread) : QObject(parent) {
  MVPN_COUNT_CTOR(PingSender);

  m_time.start();

  PingSendWorker* worker =
#if defined(MVPN_LINUX) || defined(MVPN_ANDROID)
      new LinuxPingSendWorker();
#elif defined(MVPN_MACOS) || defined(MVPN_IOS)
      new MacOSPingSendWorker();
#elif defined(MVPN_WINDOWS)
      new WindowsPingSendWorker();
#else
      new DummyPingSendWorker();
#endif

  // No multi-thread supports for wasm builds.
#ifndef MVPN_WASM
  worker->moveToThread(thread);
#endif

  connect(thread, &QThread::finished, worker, &QObject::deleteLater);
  connect(this, &PingSender::sendPing, worker, &PingSendWorker::sendPing);
  connect(this, &QObject::destroyed, worker, &QObject::deleteLater);
  connect(worker, &PingSendWorker::pingFailed, this, &PingSender::pingFailed);
  connect(worker, &PingSendWorker::pingSucceeded, this,
          &PingSender::pingSucceeded);
}

PingSender::~PingSender() { MVPN_COUNT_DTOR(PingSender); }

void PingSender::send(const QString& destination, const QString& source) {
  logger.log() << "PingSender send to" << destination;
  emit sendPing(destination, source);
}

void PingSender::pingFailed() {
  logger.log() << "PingSender - Ping Failed";
  emit completed(this, m_time.elapsed());
}

void PingSender::pingSucceeded() {
  logger.log() << "PingSender - Ping Succeeded";
  emit completed(this, m_time.elapsed());
}
