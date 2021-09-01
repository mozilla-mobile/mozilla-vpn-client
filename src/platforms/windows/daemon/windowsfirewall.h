/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSFIREWALL_H
#define WINDOWSFIREWALL_H

#include "../../daemon/interfaceconfig.h"

#include <windows.h>
#include <QString>
#include <QObject>
#include <QHostAddress>
#include <QByteArray>
class IpAdressRange;
struct FWP_VALUE0_;
struct FWP_CONDITION_VALUE0_;

class WindowsFirewall final : public QObject {
 public:
  ~WindowsFirewall();

  static WindowsFirewall* instance();
  bool init();

  bool enableKillSwitch(int vpnAdapterIndex, const InterfaceConfig& config);
  bool disableKillSwitch();

 private:
  WindowsFirewall(QObject* parent);
  HANDLE m_sessionHandle;
  bool m_init = false;
  QList<uint64_t> m_activeRules;

  QString getCurrentPath();
  void importAddress(const QHostAddress& addr, OUT FWP_VALUE0_& value);
  void importAddress(const QHostAddress& addr, OUT FWP_CONDITION_VALUE0_& value,
                     OUT QByteArray* v6DataBuffer);
  bool allowTrafficForAppOnAdapter(const QString& exePath, int networkIndex);
  bool allowTrafficForAppOnAll(const QString& exePath, int weight);
  bool blockTrafficTo(const QList<IPAddressRange>& range, uint8_t weight);
  bool blockTrafficTo(const IPAddressRange& range, uint8_t weight);
  bool blockTrafficOnPort(uint port, uint8_t weight);
  bool allowTrafficTo(const QHostAddress& targetIP, uint port, int weight);
  bool allowTrafficOfAdapter(int networkAdapter, uint8_t weight);
  bool allowDHCPTraffic(uint8_t weight);
  bool allowHyperVTraffic(uint8_t weight);
};

#endif  // WINDOWSFIREWALL_H
