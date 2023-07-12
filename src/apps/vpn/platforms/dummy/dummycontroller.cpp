/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dummycontroller.h"

#include <QHostAddress>
#include <QRandomGenerator>

#include "leakdetector.h"
#include "logger.h"
#include "models/server.h"

constexpr uint32_t DUMMY_CONNECTION_DELAY_MSEC = 1500;

namespace {
Logger logger("DummyController");
}

DummyController::DummyController() : m_delayTimer(this) {
  MZ_COUNT_CTOR(DummyController);

  m_delayTimer.setSingleShot(true);
  connect(&m_delayTimer, &QTimer::timeout, this, [&] {
    if (m_connected) {
      emit connected(m_publicKey);
    } else {
      emit disconnected();
    }
  });
}

DummyController::~DummyController() { MZ_COUNT_DTOR(DummyController); }

void DummyController::activate(const InterfaceConfig& config,
                               Controller::Reason reason) {
  Q_UNUSED(reason);

  logger.debug() << "DummyController activated" << config.m_serverIpv4AddrIn;
  logger.debug() << "DummyController DNS" << config.m_dnsServer;

  m_connected = true;
  m_publicKey = config.m_serverPublicKey;
  m_delayTimer.start(DUMMY_CONNECTION_DELAY_MSEC);
}

void DummyController::deactivate(Controller::Reason reason) {
  Q_UNUSED(reason);

  logger.debug() << "DummyController deactivated";

  m_connected = false;
  m_publicKey.clear();
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
