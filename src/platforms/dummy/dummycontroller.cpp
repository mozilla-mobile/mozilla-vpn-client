/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dummycontroller.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/server.h"
#include <QHostAddress>

#include <QRandomGenerator>

constexpr uint32_t DUMMY_CONNECTION_DELAY_MSEC = 1500;

namespace {
Logger logger(LOG_CONTROLLER, "DummyController");
}

DummyController::DummyController() : m_delayTimer(this) {
  MVPN_COUNT_CTOR(DummyController);

  m_delayTimer.setSingleShot(true);
  connect(&m_delayTimer, &QTimer::timeout, this, [&] {
    if (m_connected) {
      emit connected();
    } else {
      emit disconnected();
    }
  });
}

DummyController::~DummyController() { MVPN_COUNT_DTOR(DummyController); }

void DummyController::activate(const QList<Server>& serverList,
                               const Device* device, const Keys* keys,
                               const QList<IPAddress>& allowedIPAddressRanges,
                               const QStringList& excludedAddresses,
                               const QStringList& vpnDisabledApps,
                               const QHostAddress& dnsServer, Reason reason) {
  Q_UNUSED(device);
  Q_UNUSED(keys);
  Q_UNUSED(allowedIPAddressRanges);
  Q_UNUSED(excludedAddresses);
  Q_UNUSED(reason);
  Q_UNUSED(vpnDisabledApps);

  logger.debug() << "DummyController activated" << serverList[0].hostname();
  logger.debug() << "DummyController DNS" << dnsServer.toString();

  m_connected = true;
  m_delayTimer.start(DUMMY_CONNECTION_DELAY_MSEC);
}

void DummyController::deactivate(Reason reason) {
  Q_UNUSED(reason);

  logger.debug() << "DummyController deactivated";

  m_connected = false;
  m_delayTimer.start(DUMMY_CONNECTION_DELAY_MSEC);
}

void DummyController::checkStatus() {
  m_txBytes += QRandomGenerator::global()->generate() % 100000;
  m_rxBytes += QRandomGenerator::global()->generate() % 100000;

  emit statusUpdated("127.0.0.1", "127.0.0.1", m_txBytes, m_rxBytes);
}

void DummyController::getBackendLogs(
    std::function<void(const QString&)>&& a_callback) {
  std::function<void(const QString&)> callback = std::move(a_callback);
  callback("DummyController is always happy");
}

void DummyController::cleanupBackendLogs() {}
