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

  void excludeCgroup(const QString& cgroup);
  void resetCgroup(const QString& cgroup);
  void resetAllCgroups();

 private:
  QStringList currentInterfaces();
  bool setPeerEndpoint(struct sockaddr* sa, const QString& address, int port);
  bool addPeerPrefix(struct wg_peer* peer, const IPAddress& prefix);
  bool rtmSendRule(int action, int flags, int addrfamily);
  /**
   * This table is made up of a single routing rule:
   *    default dev moz0 proto static scope link
   *
   * This rule simply states that all packets that make it here,
   * just go through the moz0 interface.
   *
   * Packets that make it to this table must go through the Wireguard interface.
   * Firewall rules and ip rules are responsible for making sure of that.
   */
  bool setupWireguardRoutingTable();
  bool rtmIncludePeer(int action, int flags, const IPAddress& prefix);
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
