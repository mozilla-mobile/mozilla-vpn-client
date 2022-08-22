/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "serverlatency.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "pingsenderfactory.h"

#include <QDateTime>

constexpr const uint32_t SERVER_LATENCY_TIMEOUT_MSEC = 5000;

namespace {
Logger logger(LOG_MAIN, "ServerLatency");
}

ServerLatency::ServerLatency() { MVPN_COUNT_CTOR(ServerLatency); }

ServerLatency::~ServerLatency() { MVPN_COUNT_DTOR(ServerLatency); }

void ServerLatency::initialize() {
  MozillaVPN* vpn = MozillaVPN::instance();
  connect(vpn->controller(), &Controller::stateChanged, this,
          &ServerLatency::stateChanged);
}

void ServerLatency::start() {
  MozillaVPN* vpn = MozillaVPN::instance();
  if (vpn->controller()->state() != Controller::StateOff) {
    m_wantRefresh = true;
    return;
  }

  m_wantRefresh = false;
  m_pingSender = PingSenderFactory::create(QHostAddress(), this);
  ServerCountryModel* scm = vpn->serverCountryModel();

  connect(m_pingSender, SIGNAL(recvPing(quint16)), this,
          SLOT(recvPing(quint16)));
  connect(m_pingSender, SIGNAL(criticalPingError()), this,
          SLOT(criticalPingError()));
  connect(&m_timeout, &QTimer::timeout, this, &ServerLatency::stop);

  // Send a ping to every server.
  quint16 seq = 1;
  for (const Server& server : scm->servers()) {
    m_pingSender->sendPing(QHostAddress(server.ipv4AddrIn()), seq);

    ServerPingRecord record;
    record.publicKey = server.publicKey();
    record.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_pingReplyList[seq] = record;
    seq++;
  }

  m_timeout.start(SERVER_LATENCY_TIMEOUT_MSEC);
}

void ServerLatency::stop() {
  m_timeout.stop();
  m_pingReplyList.clear();

  if (m_pingSender) {
    delete m_pingSender;
    m_pingSender = nullptr;
  }
}

void ServerLatency::stateChanged() {
  Controller::State state = MozillaVPN::instance()->controller()->state();
  if (state != Controller::StateOff) {
    // If the VPN is active, then do not attempt to measure the server latency.
    stop();
  }
  else if (m_wantRefresh) {
    // If the VPN has been deactivated, start a refresh if desired.
    start();
  }
}

void ServerLatency::recvPing(quint16 sequence) {
  ServerPingRecord record = m_pingReplyList.take(sequence);
  if (record.publicKey.isEmpty()) {
    // We didn't expect this ping reply.
    return;
  }

  ServerCountryModel* scm = MozillaVPN::instance()->serverCountryModel();
  quint64 latency = QDateTime::currentMSecsSinceEpoch() - record.timestamp;
  logger.debug() << "Server" << logger.keys(record.publicKey) << "latency" << latency << "msec";
  scm->setServerLatency(record.publicKey, latency);

  if (m_pingReplyList.isEmpty()) {
    stop();
  }
}

void ServerLatency::criticalPingError() {
  logger.info() << "Encountered Unrecoverable ping error";
}
