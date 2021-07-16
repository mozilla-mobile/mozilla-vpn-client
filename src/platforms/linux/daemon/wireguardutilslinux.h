/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WIREGUARDUTILSLINUX_H
#define WIREGUARDUTILSLINUX_H

#include "daemon/wireguardutils.h"
#include <QObject>
#include <QSocketNotifier>
#include <QStringList>

class WireguardUtilsLinux final : public WireguardUtils {
  Q_OBJECT

 public:
  WireguardUtilsLinux(QObject* parent);
  ~WireguardUtilsLinux();
  bool interfaceExists() override;
  bool addInterface(const InterfaceConfig& config) override;
  bool updateInterface(const InterfaceConfig& config) override;
  bool deleteInterface() override;
  bool addRoutePrefix(const IPAddressRange& prefix) override;
  void flushRoutes() override;
  peerBytes getThroughputForInterface() override;

  QString getDefaultCgroup() const { return m_cgroups; }
  QString getExcludeCgroup() const;
  QString getBlockCgroup() const;

 private:
  QStringList currentInterfaces();
  bool setPeerEndpoint(struct sockaddr* peerEndpoint, const QString& address,
                       int port);
  bool setAllowedIpsOnPeer(struct wg_peer* peer,
                           QList<IPAddressRange> allowedIPAddressRanges);
  bool buildPeerForDevice(struct wg_device* device,
                          const InterfaceConfig& conf);
  bool setRouteRules(int action, int flags, int addrfamily);
  bool setRoutePrefix(int action, int flags, const IPAddressRange& prefix);
  static bool setupCgroupClass(const QString& path, unsigned long classid);

  int m_nlsock = -1;
  int m_nlseq = 0;
  QSocketNotifier* m_notifier = nullptr;
  QString m_cgroups;

 private slots:
  void nlsockReady();
};

#endif  // WIREGUARDUTILSLINUX_H
