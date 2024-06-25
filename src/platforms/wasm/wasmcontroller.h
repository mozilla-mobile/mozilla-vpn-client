/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WASMCONTROLLER_H
#define WASMCONTROLLER_H

#include <QDateTime>
#include <QObject>

#include "controllerimpl.h"

class MockDaemon;

class WasmController final : public ControllerImpl {
  Q_DISABLE_COPY_MOVE(WasmController)

 public:
  WasmController();
  ~WasmController();

  void initialize(const Device* device, const Keys* keys) override {
    Q_UNUSED(device);
    Q_UNUSED(keys);

    emit initialized(true, false, QDateTime());
  }

  void activate(const InterfaceConfig& config,
                Controller::Reason reason) override;

  void deactivate(Controller::Reason reason) override;

  void checkStatus() override;

  void getBackendLogs(std::function<void(const QString&)>&& callback) override;

  void cleanupBackendLogs() override;

 private:
  MockDaemon* m_mock = nullptr;
};

#endif  // WASMCONTROLLER_H
