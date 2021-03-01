/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOCALSOCKETCONTROLLER_H
#define LOCALSOCKETCONTROLLER_H

#include "controllerimpl.h"

#include <functional>
#include <QLocalSocket>

#ifdef MVPN_WINDOWS
#  include "platforms/windows/windowsservicemanager.h"
#endif

class QJsonObject;

class LocalSocketController final : public ControllerImpl {
  Q_DISABLE_COPY_MOVE(LocalSocketController)

 public:
  LocalSocketController();
  ~LocalSocketController();

  void initialize(const Device* device, const Keys* keys) override;

  void activate(const Server& server, const Device* device, const Keys* keys,
                const QList<IPAddressRange>& allowedIPAddressRanges,
                const QList<QString>& vpnDisabledApps, Reason reason) override;

  void deactivate(Reason reason) override;

  void checkStatus() override;

  void getBackendLogs(std::function<void(const QString&)>&& callback) override;

  void cleanupBackendLogs() override;

 public slots:
  void deamonReady();

 private:
  void daemonConnected();
  void errorOccurred(QLocalSocket::LocalSocketError socketError);
  void readData();
  void parseCommand(const QByteArray& command);

  void write(const QJsonObject& json);

#ifdef MVPN_WINDOWS
  WindowsServiceManager m_serviceManager;
#endif

 private:
  enum {
    eUnknown,
    eInitializing,
    eReady,
    eDisconnected,
  } m_state = eUnknown;

  QLocalSocket* m_socket = nullptr;

  QByteArray m_buffer;

  std::function<void(const QString&)> m_logCallback = nullptr;
};

#endif  // LOCALSOCKETCONTROLLER_H
