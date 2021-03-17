/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DUMMYCONTROLLER_H
#define DUMMYCONTROLLER_H

#include "controllerimpl.h"

#include <QDateTime>
#include <QObject>

class DummyController final : public ControllerImpl {
  Q_DISABLE_COPY_MOVE(DummyController)

 public:
  DummyController();
  ~DummyController();

  void initialize(const Device* device, const Keys* keys) override {
    Q_UNUSED(device);
    Q_UNUSED(keys);

    emit initialized(true, false, QDateTime());
  }

  void activate(const Server& data, const Device* device, const Keys* keys,
                const QList<IPAddressRange>& allowedIPAddressRanges,
                const QList<QString>& vpnDisabledApps, Reason reason) override;

  void deactivate(Reason reason) override;

  void checkStatus() override;

  void getBackendLogs(std::function<void(const QString&)>&& callback) override;

  void cleanupBackendLogs() override;

 private:
  int64_t m_txBytes = 0;
  int64_t m_rxBytes = 0;
};

#endif  // DUMMYCONTROLLER_H
