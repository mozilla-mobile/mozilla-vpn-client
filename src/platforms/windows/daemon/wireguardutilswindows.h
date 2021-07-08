/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WIREGUARDUTILSWINDOWS_H
#define WIREGUARDUTILSWINDOWS_H

#include "daemon/wireguardutils.h"

#include <QObject>

#include <Windows.h>

class WireguardUtilsWindows final : public WireguardUtils {
  Q_OBJECT

 public:
  WireguardUtilsWindows(QObject* parent);
  ~WireguardUtilsWindows();

  bool interfaceExists() override;
  bool addInterface(const InterfaceConfig& config) override;
  bool updateInterface(const InterfaceConfig& config) override;
  bool deleteInterface() override;
  peerBytes getThroughputForInterface() override;
  bool addRoutePrefix(const IPAddressRange& prefix) override;

 private:
  bool registerTunnelService(const QString& configFile);
  bool stopAndDeleteTunnelService(SC_HANDLE service);

 private:
  SC_HANDLE m_scm = nullptr;
  SC_HANDLE m_service = nullptr;
};

#endif  // WIREGUARDUTILSWINDOWS_H
