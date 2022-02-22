/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOCALSOCKETCONTROLLER_H
#define LOCALSOCKETCONTROLLER_H

#include "controllerimpl.h"

#include <functional>
#include <QLocalSocket>
#include <QHostAddress>
#include <QTimer>

class QJsonObject;

class LocalSocketController final : public ControllerImpl {
  Q_DISABLE_COPY_MOVE(LocalSocketController)

 public:
  LocalSocketController();
  ~LocalSocketController();

  void initialize(const Device* device, const Keys* keys) override;

  void activate(const HopConnection& hop, const Device* device,
                const Keys* keys, Reason Reason) override;

  void deactivate(Reason reason) override;

  void checkStatus() override;

  void getBackendLogs(std::function<void(const QString&)>&& callback) override;

  void cleanupBackendLogs() override;

 private:
  void initializeInternal();
  void disconnectInternal();

  void daemonConnected();
  void errorOccurred(QLocalSocket::LocalSocketError socketError);
  void readData();
  void parseCommand(const QByteArray& command);

  void write(const QJsonObject& json);

 private:
  enum {
    eUnknown,
    eInitializing,
    eReady,
    eDisconnected,
  } m_daemonState = eUnknown;

  QLocalSocket* m_socket = nullptr;

  QByteArray m_buffer;

  std::function<void(const QString&)> m_logCallback = nullptr;

  QTimer m_initializingTimer;
  uint32_t m_initializingRetry = 0;
};

#endif  // LOCALSOCKETCONTROLLER_H
