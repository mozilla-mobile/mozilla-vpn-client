/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WIREGUARDUTILSLINUX_H
#define WIREGUARDUTILSLINUX_H

#include <QHostAddress>
#include <QObject>
#include <QSocketNotifier>
#include <QStringList>

#include "daemon/wireguardutils.h"

class WireguardUtilsLinux final : public WireguardUtils {
  Q_OBJECT

 public:
  WireguardUtilsLinux(QObject* parent);
  ~WireguardUtilsLinux();
  bool interfaceExists() override;
  bool addInterface(const InterfaceConfig& config) override;
  bool deleteInterface() override;

  bool updatePeer(const InterfaceConfig& config) override;
  bool deletePeer(const InterfaceConfig& config) override;
  QList<PeerStatus> getPeerStatus() override;

  bool updateRoutePrefix(const IPAddress& prefix, int hopindex) override;
  bool deleteRoutePrefix(const IPAddress& prefix, int hopindex) override;

  bool addExclusionRoute(const QHostAddress& address) override;
  bool deleteExclusionRoute(const QHostAddress& address) override;

  void excludeCgroup(const QString& cgroup);
  void resetCgroup(const QString& cgroup);
  void resetAllCgroups();

 private:
  QStringList currentInterfaces();
  bool setPeerEndpoint(struct sockaddr* sa, const QString& address, int port);
  bool addPeerPrefix(struct wg_peer* peer, const IPAddress& prefix);
  bool rtmSendRule(int action, int flags, int addrfamily);
  bool rtmSendRoute(int action, int flags, const IPAddress& prefix,
                    int hopindex);
  bool rtmSendExclude(int action, int flags, const QHostAddress& address);
  static bool setupCgroupClass(const QString& path, unsigned long classid);
  static bool moveCgroupProcs(const QString& src, const QString& dest);
  static bool buildAllowedIp(struct wg_allowedip*, const IPAddress& prefix);

  int m_nlsock = -1;
  int m_nlseq = 0;
  QSocketNotifier* m_notifier = nullptr;

  int m_cgroupVersion = 0;
  QString m_cgroupNetClass;
  QString m_cgroupUnified;

 private slots:
  void nlsockReady();
};

#endif  // WIREGUARDUTILSLINUX_H
