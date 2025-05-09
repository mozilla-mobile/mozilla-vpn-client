/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wasmcontroller.h"

#include <QHostAddress>
#include <QJsonObject>
#include <QRandomGenerator>

#include "daemon/mock/mockdaemon.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/server.h"

namespace {
Logger logger("WasmController");
}

WasmController::WasmController() {
  MZ_COUNT_CTOR(WasmController);
  m_mock = new MockDaemon(this);

  connect(m_mock, &Daemon::disconnected, this, [&]() { emit disconnected(); });
  connect(m_mock, &Daemon::connected, this,
          [&](const QString& pubkey) { emit connected(pubkey); });
}

WasmController::~WasmController() { MZ_COUNT_DTOR(WasmController); }

void WasmController::activate(const InterfaceConfig& config,
                              Controller::Reason reason) {
  Q_UNUSED(reason);
  m_mock->activate(config);
}

void WasmController::deactivate(Controller::Reason reason) {
  Q_UNUSED(reason);
  m_mock->deactivate();
}

void WasmController::checkStatus() { emitStatusFromJson(m_mock->getStatus()); }
