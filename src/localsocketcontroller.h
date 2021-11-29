/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOCALSOCKETCONTROLLER_H
#define LOCALSOCKETCONTROLLER_H

#include "controllerimpl.h"

#include <functional>
#include <QLocalSocket>
#include <QHostAddress>

class QJsonObject;

class LocalSocketController final : public ControllerImpl {
  Q_DISABLE_COPY_MOVE(LocalSocketController)

 public:
  LocalSocketController();
  ~LocalSocketController();

  void initialize(const Device* device, const Keys* keys) override;

  void activate(const QList<Server>& serverList, const Device* device,
                const Keys* keys,
                const QList<IPAddressRange>& allowedIPAddressRanges,
                const QList<QString>& vpnDisabledApps,
                const QHostAddress& dnsServer, Reason reason) override;

  void deactivate(Reason reason) override;

  void checkStatus() override;

  void getBackendLogs(std::function<void(const QString&)>&& callback) override;

  void cleanupBackendLogs() override;

 private:
  void activateNext();
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
  } m_state = eUnknown;

  class HopConnection {
   public:
    HopConnection() {}
    Server m_server;
    int m_hopindex = 0;
    QList<IPAddressRange> m_allowedIPAddressRanges;
    QList<QString> m_vpnDisabledApps;
    QHostAddress m_dnsServer;
  };
  QList<HopConnection> m_activationQueue;
  const Device* m_device = nullptr;
  const Keys* m_keys = nullptr;

  QLocalSocket* m_socket = nullptr;

  QByteArray m_buffer;

  std::function<void(const QString&)> m_logCallback = nullptr;
};

#endif  // LOCALSOCKETCONTROLLER_H
