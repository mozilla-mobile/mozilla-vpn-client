/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "pinghelper.h"
#include "leakdetector.h"
#include "logger.h"
#include "pingsender.h"

// Any X seconds, a new ping.
constexpr uint32_t PING_TIMOUT_SEC = 1;

// How many concurrent pings
constexpr int PINGS_MAX = 20;

namespace {
Logger logger(LOG_NETWORKING, "PingHelper");
}

PingHelper::PingHelper() {
  MVPN_COUNT_CTOR(PingHelper);

  connect(&m_pingTimer, &QTimer::timeout, this, &PingHelper::nextPing);

  m_pingThread.start();
}

PingHelper::~PingHelper() {
  MVPN_COUNT_DTOR(PingHelper);

  m_pingThread.quit();
  m_pingThread.wait();
}

void PingHelper::start(const QString& serverIpv4Gateway,
                       const QString& deviceIpv4Address) {
  logger.log() << "PingHelper activated for server:" << serverIpv4Gateway;

  m_gateway = serverIpv4Gateway;
  m_source = deviceIpv4Address.section('/', 0, 0);
  m_pingTimer.start(PING_TIMOUT_SEC * 1000);
}

void PingHelper::stop() {
  logger.log() << "PingHelper deactivated";

  m_pingTimer.stop();

  for (PingSender* pingSender : m_pings) {
    pingSender->deleteLater();
  }
  m_pings.clear();
}

void PingHelper::nextPing() {
  logger.log() << "Sending a new ping. Total:" << m_pings.length();

  PingSender* pingSender = new PingSender(this, &m_pingThread);
  connect(pingSender, &PingSender::completed, this, &PingHelper::pingReceived);
  m_pings.append(pingSender);
  pingSender->send(m_gateway, m_source);

  while (m_pings.length() > PINGS_MAX) {
    m_pings.at(0)->deleteLater();
    m_pings.removeAt(0);
  }
}

void PingHelper::pingReceived(PingSender* pingSender, qint64 msec) {
  logger.log() << "Ping answer received in msec:" << msec;

  if (!m_pingTimer.isActive()) {
    logger.log() << "Race condition. Let's ignore this ping";
    return;
  }

  QMutableListIterator<PingSender*> i(m_pings);
  while (i.hasNext()) {
    PingSender* thisPingSender = i.next();
    if (thisPingSender != pingSender) {
      continue;
    }

    i.remove();
    break;
  }

  pingSender->deleteLater();
  emit pingSentAndReceived(msec);
}
