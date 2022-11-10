/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "serverlatency.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/feature.h"
#include "mozillavpn.h"
#include "pingsenderfactory.h"

#include <QDateTime>

constexpr const uint32_t SERVER_LATENCY_TIMEOUT_MSEC = 5000;

constexpr const uint32_t SERVER_LATENCY_INITIAL_MSEC = 1000;

constexpr const uint32_t SERVER_LATENCY_REFRESH_MSEC = 1800000;

constexpr const int SERVER_LATENCY_MAX_PARALLEL = 8;

constexpr const int SERVER_LATENCY_MAX_RETRIES = 2;

namespace {
Logger logger(LOG_MAIN, "ServerLatency");
}

ServerLatency::ServerLatency() { MVPN_COUNT_CTOR(ServerLatency); }

ServerLatency::~ServerLatency() { MVPN_COUNT_DTOR(ServerLatency); }

void ServerLatency::initialize() {
  MozillaVPN* vpn = MozillaVPN::instance();

  connect(vpn->serverCountryModel(), &ServerCountryModel::changed, this,
          &ServerLatency::start);

  connect(vpn->controller(), &Controller::stateChanged, this,
          &ServerLatency::stateChanged);

  connect(&m_pingTimeout, &QTimer::timeout, this,
          &ServerLatency::maybeSendPings);

  connect(&m_refreshTimer, &QTimer::timeout, this, &ServerLatency::start);

  m_refreshTimer.start(SERVER_LATENCY_INITIAL_MSEC);
}

void ServerLatency::start() {
  if (!Feature::get(Feature::Feature_serverConnectionScore)->isSupported()) {
    return;
  }

  MozillaVPN* vpn = MozillaVPN::instance();
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
  ServerCountryModel* scm = vpn->serverCountryModel();

  connect(m_pingSender, SIGNAL(recvPing(quint16)), this,
          SLOT(recvPing(quint16)));
  connect(m_pingSender, SIGNAL(criticalPingError()), this,
          SLOT(criticalPingError()));

  // Generate a list of servers to ping.
  // TODO: Could be m_pingSendQueue = scm->m_servers.keys(), but it's private.
  for (const Server& server : scm->servers()) {
    m_pingSendQueue.append(server.publicKey());
  }

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

      Server server = scm->server(retry.publicKey);
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

    ServerPingRecord record;
    record.publicKey = m_pingSendQueue.takeFirst();
    record.sequence = m_sequence++;
    record.timestamp = now;
    record.retries = 0;
    m_pingReplyList.append(record);

    Server server = scm->server(record.publicKey);
    m_pingSender->sendPing(QHostAddress(server.ipv4AddrIn()), record.sequence);
  }

  if (m_pingReplyList.isEmpty()) {
    // If the ping reply list is empty, then we have nothing left to do.
    stop();
  } else {
    // Otherwise, the list should be sorted by transmit time. Schedule a timer
    // to cleanup anything that experiences a timeout.
    const ServerPingRecord& record = m_pingReplyList.first();
    m_pingTimeout.start(SERVER_LATENCY_TIMEOUT_MSEC - (now - record.timestamp));
  }
}

void ServerLatency::stop() {
  m_pingTimeout.stop();
  m_pingSendQueue.clear();
  m_pingReplyList.clear();

  if (m_pingSender) {
    delete m_pingSender;
    m_pingSender = nullptr;
  }

  if (!m_refreshTimer.isActive()) {
    m_refreshTimer.start(SERVER_LATENCY_REFRESH_MSEC);
  }
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
  for (auto i = m_pingReplyList.begin(); i != m_pingReplyList.end(); i++) {
    const ServerPingRecord& record = *i;
    if (record.sequence != sequence) {
      continue;
    }

    ServerCountryModel* scm = MozillaVPN::instance()->serverCountryModel();
    quint64 latency = QDateTime::currentMSecsSinceEpoch() - record.timestamp;
    scm->setServerLatency(record.publicKey, latency);

    m_pingReplyList.erase(i);
    maybeSendPings();
    return;
  }
}

void ServerLatency::criticalPingError() {
  logger.info() << "Encountered Unrecoverable ping error";
}
