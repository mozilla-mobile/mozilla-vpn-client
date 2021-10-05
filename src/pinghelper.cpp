/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "pinghelper.h"
#include "leakdetector.h"
#include "logger.h"
#include "pingsender.h"
#include "platforms/dummy/dummypingsender.h"
#include "timersingleshot.h"

#if defined(MVPN_LINUX) || defined(MVPN_ANDROID)
#  include "platforms/linux/linuxpingsender.h"
#elif defined(MVPN_MACOS) || defined(MVPN_IOS)
#  include "platforms/macos/macospingsender.h"
#elif defined(MVPN_WINDOWS)
#  include "platforms/windows/windowspingsender.h"
#elif defined(MVPN_DUMMY) || defined(UNIT_TEST)
#else
#  error "Unsupported platform"
#endif

#include <QDateTime>

#include <cmath>

// Any X seconds, a new ping.
constexpr uint32_t PING_TIMOUT_SEC = 1;

// Maximum window size for ping statistics.
constexpr int PING_STATS_WINDOW = 32;

namespace {
Logger logger(LOG_NETWORKING, "PingHelper");
bool s_has_critical_ping_error = false;
}  // namespace

PingHelper::PingHelper() {
  MVPN_COUNT_CTOR(PingHelper);

  m_sequence = 0;
  m_pingData.resize(PING_STATS_WINDOW);

  connect(&m_pingTimer, &QTimer::timeout, this, &PingHelper::nextPing);
}

PingHelper::~PingHelper() { MVPN_COUNT_DTOR(PingHelper); }

void PingHelper::start(const QString& serverIpv4Gateway,
                       const QString& deviceIpv4Address) {
  logger.debug() << "PingHelper activated for server:" << serverIpv4Gateway;

  m_gateway = serverIpv4Gateway;
  m_source = deviceIpv4Address.section('/', 0, 0);

  if (s_has_critical_ping_error) {
    m_pingSender = new DummyPingSender(m_source, this);
  } else {
    m_pingSender =
#if defined(MVPN_LINUX) || defined(MVPN_ANDROID)
        new LinuxPingSender(m_source, this);
#elif defined(MVPN_MACOS) || defined(MVPN_IOS)
        new MacOSPingSender(m_source, this);
#elif defined(MVPN_WINDOWS)
        new WindowsPingSender(m_source, this);
#else
        new DummyPingSender(m_source, this);
#endif
  }
  connect(m_pingSender, &PingSender::recvPing, this, &PingHelper::pingReceived);
  connect(m_pingSender, &PingSender::criticalPingError, this,
          &PingHelper::handlePingError);

  // Reset the ping statistics
  m_sequence = 0;
  for (int i = 0; i < PING_STATS_WINDOW; i++) {
    m_pingData[i].timestamp = -1;
    m_pingData[i].latency = -1;
    m_pingData[i].sequence = 0;
  }

  m_pingTimer.start(PING_TIMOUT_SEC * 1000);
}

void PingHelper::stop() {
  logger.debug() << "PingHelper deactivated";

  if (m_pingSender) {
    delete m_pingSender;
    m_pingSender = nullptr;
  }

  m_pingTimer.stop();
}

void PingHelper::nextPing() {
#ifdef QT_DEBUG
  logger.debug() << "Sending ping seq:" << m_sequence;
#endif

  // The ICMP sequence number is used to match replies with their originating
  // request, and serves as an index into the circular buffer. Overflows of
  // the sequence number acceptable.
  int index = m_sequence % PING_STATS_WINDOW;
  m_pingData[index].timestamp = QDateTime::currentMSecsSinceEpoch();
  m_pingData[index].latency = -1;
  m_pingData[index].sequence = m_sequence;
  m_pingSender->sendPing(m_gateway, m_sequence);

  m_sequence++;
}

void PingHelper::pingReceived(quint16 sequence) {
  int index = sequence % PING_STATS_WINDOW;
  if (m_pingData[index].sequence == sequence) {
    qint64 sendTime = m_pingData[index].timestamp;
    m_pingData[index].latency = QDateTime::currentMSecsSinceEpoch() - sendTime;
    emit pingSentAndReceived(m_pingData[index].latency);
#ifdef QT_DEBUG
    logger.debug() << "Ping answer received seq:" << sequence
                   << "avg:" << latency()
                   << "loss:" << QString("%1%").arg(loss() * 100.0)
                   << "stddev:" << stddev();
#endif
  }
}

uint PingHelper::latency() const {
  int recvCount = 0;
  qint64 totalMsec = 0;

  for (const PingSendData& data : m_pingData) {
    if (data.latency < 0) {
      continue;
    }
    recvCount++;
    totalMsec += data.latency;
  }

  if (recvCount <= 0) {
    return 0.0;
  }

  // Add half the denominator to produce nearest-integer rounding.
  totalMsec += recvCount / 2;
  return static_cast<uint>(totalMsec / recvCount);
}

uint PingHelper::stddev() const {
  int recvCount = 0;
  qint64 totalVariance = 0;
  uint average = PingHelper::latency();

  for (const PingSendData& data : m_pingData) {
    if (data.latency < 0) {
      continue;
    }
    recvCount++;
    totalVariance += (average - data.latency) * (average - data.latency);
  }

  if (recvCount <= 0) {
    return 0.0;
  }

  return std::sqrt((double)totalVariance / recvCount);
}

uint PingHelper::maximum() const {
  uint maxRtt = 0;

  for (const PingSendData& data : m_pingData) {
    if (data.latency < 0) {
      continue;
    }
    if (data.latency > maxRtt) {
      maxRtt = data.latency;
    }
  }
  return maxRtt;
}

double PingHelper::loss() const {
  int sendCount = 0;
  int recvCount = 0;
  // Don't count pings that are possibly still in flight as losses.
  qint64 sendBefore =
      QDateTime::currentMSecsSinceEpoch() - (PING_TIMOUT_SEC * 1000);

  for (const PingSendData& data : m_pingData) {
    if (data.latency >= 0) {
      recvCount++;
      sendCount++;
    } else if ((data.timestamp > 0) && (data.timestamp < sendBefore)) {
      sendCount++;
    }
  }

  if (sendCount <= 0) {
    return 0.0;
  }
  return (double)(sendCount - recvCount) / sendCount;
}

void PingHelper::handlePingError() {
  if (s_has_critical_ping_error) {
    return;
  }
  logger.info() << "Encountered Unrecoverable ping error, switching to DUMMY "
                   "Ping for next 10 Minutes";
  // When the ping helper is unable to work, set the error flag
  // and restart the pinghelper, to replace the impl with a dummy impl
  // which we will use for the rest of the session
  emit pingSentAndReceived(1);  // Fake a ping response with 1ms;
  s_has_critical_ping_error = true;
  stop();
  start(m_gateway, m_source);

  TimerSingleShot::create(this, 600000, [&] {  // 10 Minutes
    logger.debug() << "Removing ping error state";
    s_has_critical_ping_error = false;
    if (m_pingSender) {
      stop();
      start(m_gateway, m_source);
    }
  });
}
