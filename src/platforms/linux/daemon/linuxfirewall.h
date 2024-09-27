/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LINUXFIREWALL_H
#define LINUXFIREWALL_H

#include <QObject>

class LinuxFirewall final : public QObject {
  Q_DISABLE_COPY_MOVE(LinuxFirewall)

 public:
  ~LinuxFirewall();
  LinuxFirewall(QObject* parent);

  bool up(const QString& ifname, uint32_t fwmark,
          const QString& deviceIpv6Address);
  bool down();

  bool markInbound(const QString& serverIpv4AddrIn);
  bool clearInbound(const QString& serverIpv4AddrIn);

  bool markCgroupV1(uint32_t cgroup);
  bool markCgroupV2(const QString& cgroup);
  bool clearCgroupV2(const QString& cgroup);
  bool clearAllCgroupsV2();

 private:
  bool m_isUp = false;
};

#endif  // LINUXFIREWALL_H
