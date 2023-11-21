/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "serverlatency.h"

#include <QDateTime>

#include "controller.h"
#include "feature.h"
#include "leakdetector.h"
#include "logger.h"
#include "mfbt/checkedint.h"
#include "models/location.h"
#include "models/servercountrymodel.h"
#include "mozillavpn.h"
#include "pingsenderfactory.h"

constexpr const uint32_t SERVER_LATENCY_TIMEOUT_MSEC = 5000;

constexpr const uint32_t SERVER_LATENCY_INITIAL_MSEC = 1000;

constexpr const uint32_t SERVER_LATENCY_REFRESH_MSEC = 1800000;

constexpr const int SERVER_LATENCY_MAX_PARALLEL = 8;

constexpr const int SERVER_LATENCY_MAX_RETRIES = 2;

// Delay the progressChanged() signal to rate-limit how often score changes.
constexpr const uint32_t SERVER_LATENCY_PROGRESS_DELAY_MSEC = 500;

// Minimum number of redundant servers we expect at a location.
constexpr int SCORE_SERVER_REDUNDANCY_THRESHOLD = 3;

namespace {
Logger logger("ServerLatency");
}

ServerLatency::ServerLatency() { MZ_COUNT_CTOR(ServerLatency); }

ServerLatency::~ServerLatency() { MZ_COUNT_DTOR(ServerLatency); }

void ServerLatency::initialize() {
  MozillaVPN* vpn = MozillaVPN::instance();

  connect(vpn->serverCountryModel(), &ServerCountryModel::changed, this,
          &ServerLatency::start);

  connect(vpn->controller(), &Controller::stateChanged, this,
          &ServerLatency::stateChanged);

  connect(&m_pingTimeout, &QTimer::timeout, this,
          &ServerLatency::maybeSendPings);

  m_refreshTimer.setSingleShot(true);
  connect(&m_refreshTimer, &QTimer::timeout, this, &ServerLatency::start);

  m_progressDelayTimer.setSingleShot(true);
  connect(&m_progressDelayTimer, &QTimer::timeout, this,
          [this]() { emit progressChanged(); });

  const Feature* feature = Feature::get(Feature::Feature_serverConnectionScore);
  connect(feature, &Feature::supportedChanged, this, &ServerLatency::start);
  if (feature->isSupported()) {
    m_refreshTimer.start(SERVER_LATENCY_INITIAL_MSEC);
  }
}

void ServerLatency::start() {
  MozillaVPN* vpn = MozillaVPN::instance();
  if (!Feature::get(Feature::Feature_serverConnectionScore)->isSupported()) {
    clear();
    return;
  }

  if (vpn->controller()->state() != Controller::StateOff) {
    // Don't attempt to refresh latency when the VPN is active, or
    // we could get misleading results.
    m_wantRefresh = true;
    return;
  }
  if (m_pingSender != nullptr) {
    // Don't start a latency refresh if one is already in progress.
    return;
  }

  m_sequence = 0;
  m_wantRefresh = false;
  m_pingSender = PingSenderFactory::create(QHostAddress(), this);

  connect(m_pingSender, SIGNAL(recvPing(quint16)), this,
          SLOT(recvPing(quint16)), Qt::QueuedConnection);
  connect(m_pingSender, SIGNAL(criticalPingError()), this,
          SLOT(criticalPingError()));

  // Generate a list of servers to ping. If possible, sort them by geographic
  // distance to try and get data for the quickest servers first.
  for (const ServerCountry& country : vpn->serverCountryModel()->countries()) {
    for (const QString& cityName : country.cities()) {
      const ServerCity& city =
          vpn->serverCountryModel()->findCity(country.code(), cityName);
      double distance =
          vpn->location()->distance(city.latitude(), city.longitude());
      Q_ASSERT(city.initialized());

      // Search for where in the list to insert this city's servers.
      auto i = m_pingSendQueue.begin();
      while (i != m_pingSendQueue.end()) {
        if (i->distance >= distance) {
          break;
        }
        i++;
      }

      // Insert the servers into the list.
      for (const QString& pubkey : city.servers()) {
        ServerPingRecord rec = {
            pubkey, city.country(), city.name(), 0, 0, distance, 0};
        i = m_pingSendQueue.insert(i, rec);
      }
    }
  }

  m_pingSendTotal = m_pingSendQueue.count();

  m_progressDelayTimer.stop();
  emit progressChanged();

  m_refreshTimer.stop();
  maybeSendPings();
}

void ServerLatency::maybeSendPings() {
  quint64 now = QDateTime::currentMSecsSinceEpoch();
  ServerCountryModel* scm = MozillaVPN::instance()->serverCountryModel();
  if (m_pingSender == nullptr) {
    return;
  }

  // Scan through the reply list, looking for timeouts.
  while (!m_pingReplyList.isEmpty()) {
    const ServerPingRecord& record = m_pingReplyList.first();
    if ((record.timestamp + SERVER_LATENCY_TIMEOUT_MSEC) > now) {
      break;
    }
    logger.debug() << "Server" << logger.keys(record.publicKey) << "timeout"
                   << record.retries;

    // Send a retry.
    if (record.retries < SERVER_LATENCY_MAX_RETRIES) {
      ServerPingRecord retry = record;
      retry.retries++;
      retry.sequence = m_sequence++;
      retry.timestamp = now;
      m_pingReplyList.append(retry);

      const Server& server = scm->server(retry.publicKey);
      m_pingSender->sendPing(QHostAddress(server.ipv4AddrIn()), retry.sequence);
    }

    // TODO: Mark the server unavailable?
    m_pingReplyList.removeFirst();
  }

  // Generate new pings until we reach our max number of parallel pings.
  while (m_pingReplyList.count() < SERVER_LATENCY_MAX_PARALLEL) {
    if (m_pingSendQueue.isEmpty()) {
      break;
    }

    ServerPingRecord record = m_pingSendQueue.takeFirst();
    record.sequence = m_sequence++;
    record.timestamp = now;
    record.retries = 0;
    m_pingReplyList.append(record);

    const Server& server = scm->server(record.publicKey);
    m_pingSender->sendPing(QHostAddress(server.ipv4AddrIn()), record.sequence);
  }

  m_lastUpdateTime = QDateTime::currentDateTime();
  if (!m_progressDelayTimer.isActive()) {
    m_progressDelayTimer.start(SERVER_LATENCY_PROGRESS_DELAY_MSEC);
  }

  if (m_pingReplyList.isEmpty()) {
    // If the ping reply list is empty, then we have nothing left to do.
    stop();
  } else {
    // Otherwise, the list should be sorted by transmit time. Schedule a timer
    // to cleanup anything that experiences a timeout.
    const ServerPingRecord& record = m_pingReplyList.first();

    CheckedInt<int> value(SERVER_LATENCY_TIMEOUT_MSEC);
    value -= static_cast<int>(now - record.timestamp);

    m_pingTimeout.start(value.value());
  }
}

void ServerLatency::stop() {
  m_pingTimeout.stop();
  m_pingSendQueue.clear();
  m_pingReplyList.clear();
  m_pingSendTotal = 0;

  if (m_pingSender) {
    m_pingSender->deleteLater();
    m_pingSender = nullptr;
  }

  emit progressChanged();
  m_progressDelayTimer.stop();
  if (!m_refreshTimer.isActive()) {
    m_refreshTimer.start(SERVER_LATENCY_REFRESH_MSEC);
  }
}

void ServerLatency::refresh() {
  if (m_pingSender) {
    return;
  }

  clear();
  start();
}

void ServerLatency::clear() {
  m_latency.clear();
  m_sumLatencyMsec = 0;

  emit progressChanged();
}

void ServerLatency::stateChanged() {
  Controller::State state = MozillaVPN::instance()->controller()->state();
  if (state != Controller::StateOff) {
    // If the VPN is active, then do not attempt to measure the server latency.
    stop();
  } else if (m_wantRefresh) {
    // If the VPN has been deactivated, start a refresh if desired.
    start();
  }
}

void ServerLatency::recvPing(quint16 sequence) {
  qint64 now(QDateTime::currentMSecsSinceEpoch());

  for (auto i = m_pingReplyList.begin(); i != m_pingReplyList.end(); i++) {
    const ServerPingRecord& record = *i;
    if (record.sequence != sequence) {
      continue;
    }

    ServerCountryModel* scm = MozillaVPN::instance()->serverCountryModel();

    qint64 latency(now - record.timestamp);
    if (latency <= std::numeric_limits<uint>::max()) {
      setLatency(record.publicKey, latency);

      const ServerCity& city =
          scm->findCity(record.countryCode, record.cityName);
      if (city.initialized()) {
        emit city.scoreChanged();
      }
    }

    m_pingReplyList.erase(i);
    maybeSendPings();
    return;
  }
}

void ServerLatency::criticalPingError() {
  logger.info() << "Encountered Unrecoverable ping error";
}

qint64 ServerLatency::avgLatency() const {
  if (m_latency.isEmpty()) {
    return 0;
  }
  return (m_sumLatencyMsec + m_latency.count() - 1) / m_latency.count();
}

void ServerLatency::setLatency(const QString& pubkey, qint64 msec) {
  m_sumLatencyMsec -= m_latency[pubkey];
  m_sumLatencyMsec += msec;
  m_latency[pubkey] = msec;
}

double ServerLatency::progress() const {
  if ((m_pingSender == nullptr) || (m_pingSendTotal == 0)) {
    return 1.0;  // Operation is complete.
  }

  double remaining = m_pingReplyList.count() + m_pingSendQueue.count();
  return 1.0 - (remaining / m_pingSendTotal);
}

void ServerLatency::setCooldown(const QString& publicKey, qint64 timeout) {
  if (timeout <= 0) {
    m_cooldown.remove(publicKey);
  } else {
    m_cooldown[publicKey] = QDateTime::currentSecsSinceEpoch() + timeout;
  }

  // Emit signals that the connection score may have changed.
  ServerCountryModel* scm = MozillaVPN::instance()->serverCountryModel();
  const Server& server = scm->server(publicKey);
  const ServerCity& city =
      scm->findCity(server.countryCode(), server.cityName());
  if (city.initialized()) {
    emit city.scoreChanged();
  }
}

int ServerLatency::baseCityScore(const ServerCity* city,
                                 const QString& originCountry) const {
  qint64 now = QDateTime::currentSecsSinceEpoch();
  int score = Poor;
  int activeServerCount = 0;
  for (const QString& pubkey : city->servers()) {
    if (getCooldown(pubkey) <= now) {
      activeServerCount++;
    }
  }

  // Ensure there is at least one reachable server.
  if (activeServerCount == 0) {
    return Unavailable;
  }

  // Increase the score if the location has sufficient redundancy.
  if (activeServerCount >= SCORE_SERVER_REDUNDANCY_THRESHOLD) {
    score++;
  }

  // Increase the score for connections made within the same country.
  if ((!originCountry.isEmpty()) &&
      (originCountry.compare(city->country(), Qt::CaseInsensitive) == 0)) {
    score++;
  }

  if (score > Excellent) {
    score = Excellent;
  }
  return score;
}
