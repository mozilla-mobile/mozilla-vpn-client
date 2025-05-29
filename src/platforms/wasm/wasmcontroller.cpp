/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wasmcontroller.h"

#include <QHostAddress>
#include <QJsonDocument>
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
  QJsonDocument jsDocument = QJsonDocument(config.toJson());
  QByteArray jsBlob = jsDocument.toJson(QJsonDocument::Compact);
  QMetaObject::invokeMethod(m_mock, "activate", Qt::QueuedConnection,
                            Q_ARG(QString, QString::fromUtf8(jsBlob)));
}

void WasmController::deactivate(Controller::Reason reason) {
  Q_UNUSED(reason);
  QMetaObject::invokeMethod(m_mock, "deactivate", Qt::QueuedConnection);
}

void WasmController::checkStatus() { emitStatusFromJson(m_mock->getStatus()); }
