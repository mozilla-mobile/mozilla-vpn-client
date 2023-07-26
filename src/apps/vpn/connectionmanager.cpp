/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectionmanager.h"

#include "controller.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkwatcher.h"

namespace {
Logger logger("Connection Manager");
}

ConnectionManager::ConnectionManager() {
  MZ_COUNT_CTOR(ConnectionManager);
  
  m_connectingTimer.setSingleShot(true);
  m_handshakeTimer.setSingleShot(true);

  connect(&m_timer, &QTimer::timeout, this, &ConnectionManager::timerTimeout);

  connect(&m_connectingTimer, &QTimer::timeout, this, [this]() {
    m_enableDisconnectInConfirming = true;
    emit enableDisconnectInConfirmingChanged();
  });

  connect(&m_handshakeTimer, &QTimer::timeout, this,
          &ConnectionManager::handshakeTimeout);

//  LogHandler::instance()->registerLogSerializer(this);
}

ConnectionManager::~ConnectionManager() {
  MZ_COUNT_DTOR(ConnectionManager);
}

void ConnectionManager::initialize() {
  logger.debug() << "Initializing the connection manager";
}

qint64 ConnectionManager::time() const {
  if (m_connectedTimeInUTC.isValid()) {
    return m_connectedTimeInUTC.secsTo(QDateTime::currentDateTimeUtc());
  }
  return 0;
}

void ConnectionManager::timerTimeout() {
  Q_ASSERT(m_state == StateOn);
  emit timeChanged();
}

void ConnectionManager::handshakeTimeout() {
  logger.debug() << "Timeout while waiting for handshake";

//  MozillaVPN* vpn = MozillaVPN::instance();
//  Q_ASSERT(!m_activationQueue.isEmpty());
//
//  // Block the offending server and try again.
//  InterfaceConfig& hop = m_activationQueue.first();
//  vpn->serverLatency()->setCooldown(
//      hop.m_serverPublicKey, AppConstants::SERVER_UNRESPONSIVE_COOLDOWN_SEC);
//
//  emit handshakeFailed(hop.m_serverPublicKey);
//
//  if (m_nextStep != None) {
//    deactivate();
//    return;
//  }
//
//  // Try again, again if there are sufficient retries left.
//  ++m_connectionRetry;
//  emit connectionRetryChanged();
//  logger.info() << "Connection attempt " << m_connectionRetry;
//  if (m_connectionRetry == 1) {
//    logger.info() << "Connection Attempt: Using Port 53 Option this time.";
//    // On the first retry, opportunisticly try again using the port 53
//    // option enabled, if that feature is disabled.
//    activateInternal(ForceDNSPort, RandomizeServerSelection);
//    return;
//  } else if (m_connectionRetry < CONNECTION_MAX_RETRY) {
//    activateInternal(DoNotForceDNSPort, RandomizeServerSelection);
//    return;
//  }
//
//  // Otherwise, the give up and report the location as unavailable.
//  logger.error() << "Connection retries exhausted, giving up";
//  serverUnavailable();
}
