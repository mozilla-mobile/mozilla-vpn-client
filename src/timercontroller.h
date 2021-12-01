/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TIMERCONTROLLER_H
#define TIMERCONTROLLER_H

#include "controllerimpl.h"

#include <QObject>
#include <QTimer>
#include <QHostAddress>

constexpr uint32_t TIME_ACTIVATION = 1000;
constexpr uint32_t TIME_DEACTIVATION = 1500;
constexpr uint32_t TIME_SWITCHING = 2000;
constexpr uint32_t TIME_CONFIRMING = 0;

class TimerController final : public ControllerImpl {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(TimerController)

 public:
  TimerController(ControllerImpl* impl);
  ~TimerController();

  void initialize(const Device* device, const Keys* keys) override;

  void activate(const QList<Server>& serverList, const Device* device,
                const Keys* keys,
                const QList<IPAddress>& allowedIPAddressRanges,
                const QStringList& excludedAddresses,
                const QStringList& vpnDisabledApps, const QHostAddress& dns,
                Reason reason) override;

  void deactivate(Reason reason) override;

  void checkStatus() override;

  void getBackendLogs(std::function<void(const QString&)>&& callback) override;

  void cleanupBackendLogs() override;

 private slots:
  void timeout();

 private:
  void maybeDone(bool isConnected);

 private:
  ControllerImpl* m_impl;
  QTimer m_timer;

  enum State {
    None,
    Connecting,
    Connected,
    Disconnecting,
    Disconnected,
  };

  State m_state = None;
};

#endif  // TIMERCONTROLLER_H
