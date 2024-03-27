/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxfirewall.h"

#include "controller.h"
#include "leakdetector.h"
#include "logger.h"

// Import Netfilter.go headers
#if defined(__cplusplus)
extern "C" {
#endif
#include "netfilter.h"
#if defined(__cplusplus)
}
#endif

namespace {
Logger netfilterLogger("NetfilterGo");
Logger logger("LinuxFirewall");

void NetfilterLogger(int level, const char* msg) {
  // https:// pkg.go.dev/github.com/tusharsoni/copper/clogger#pkg-constants
  //
  // LevelDebug = Level(iota + 1)
  // LevelInfo
  // LevelWarn
  // LevelError

  switch (level) {
    case 2:
      netfilterLogger.info() << msg;
    case 3:
      netfilterLogger.warning() << msg;
    case 4:
      netfilterLogger.error() << msg;
    default:
      netfilterLogger.debug() << msg;
  }
}

// Using a macro instead of a function here,
// because we want the QByteArray to have the same lifetime as the caller.
#define MAKE_GO_STRING(identifier, str)                   \
  QByteArray b_##identifier = str.toLocal8Bit();          \
  GoString identifier = {.p = b_##identifier.constData(), \
                         .n = (ptrdiff_t)b_##identifier.length()};

}  // namespace

LinuxFirewall::LinuxFirewall(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(LinuxFirewall);

  NetfilterSetLogger((GoUintptr)&NetfilterLogger);
}

LinuxFirewall::~LinuxFirewall() {
  MZ_COUNT_DTOR(LinuxFirewall);

  down();
}

bool LinuxFirewall::up(const QString& ifname, uint32_t fwmark,
                       const QString& deviceIpv6Address) {
  logger.debug() << "Starting firewall";

  if (NetfilterCreateTables() != 0) {
    return false;
  }

  // Once tables exist there is something to clean up, therefore we are up.
  m_isUp = true;
  // TODO: We should have a better solution for this once VPN-6256 is addressed.
  auto cleanup = qScopeGuard([this] {
    logger.error() << "Error attempting to setup firewall.";
    down();
  });

  if (NetfilterApplyFwMark(fwmark) != 0) {
    return false;
  }

  MAKE_GO_STRING(goIfname, ifname);
  if (NetfilterRestrictTraffic(goIfname) != 0) {
    return false;
  }

  int slashPos = deviceIpv6Address.indexOf('/');
  MAKE_GO_STRING(goIpv6Address, deviceIpv6Address);
  if (slashPos != -1) {
    goIpv6Address.n = slashPos;
  }

  if (NetfilterIsolateIpv6(goIfname, goIpv6Address)) {
    return false;
  }

  for (const IPAddress& prefix :
       Controller::getExcludedIPAddressRanges().flatten()) {
    MAKE_GO_STRING(goPrefix, prefix.toString());
    if (NetfilterAllowPrefix(goPrefix) != 0) {
      return false;
    }
  }

  if (NetfilterAllowDHCP() != 0) {
    return false;
  }

  if (NetfilterBlockDNS() != 0) {
    return false;
  }

  cleanup.dismiss();
  return true;
}

bool LinuxFirewall::down() {
  logger.debug() << "Disabling firewall";

  if (m_isUp && NetfilterRemoveTables() != 0) {
    // TODO(VPN-6265): Should we retry?
    logger.error() << "Error attempting to disable firewall.";
    return false;
  }

  return true;
}

bool LinuxFirewall::markInbound(const QString& serverIpv4AddrIn) {
  MAKE_GO_STRING(goAddress, serverIpv4AddrIn);
  if (NetfilterMarkInbound(goAddress) != 0) {
    logger.error() << "Error attempting to mark inbound traffic from"
                   << serverIpv4AddrIn;
    return false;
  }

  return true;
}

bool LinuxFirewall::clearInbound(const QString& serverIpv4AddrIn) {
  MAKE_GO_STRING(goAddress, serverIpv4AddrIn);
  if (NetfilterClearInbound(goAddress) != 0) {
    logger.error() << "Error attempting to clear inbound traffic from"
                   << serverIpv4AddrIn;
    return false;
  }

  return true;
}

bool LinuxFirewall::markCgroupV1(uint32_t cgroup) {
  if (NetfilterMarkCgroupV1(cgroup) != 0) {
    logger.error() << "Error attempting to mark cgroupv1 traffic for cgroup"
                   << cgroup;
    return false;
  }

  return true;
}

bool LinuxFirewall::markCgroupV2(const QString& cgroup) {
  MAKE_GO_STRING(goCgroup, cgroup);
  if (NetfilterMarkCgroupV2(goCgroup) != 0) {
    logger.error() << "Error attempting to mark cgroupv2 traffic for cgroup"
                   << cgroup;
    return false;
  }

  return true;
}

bool LinuxFirewall::clearCgroupV2(const QString& cgroup) {
  MAKE_GO_STRING(goCgroup, cgroup);
  if (NetfilterResetCgroupV2(goCgroup) != 0) {
    logger.error() << "Error attempting to clear cgroupv2 traffic for cgroup"
                   << cgroup;
    return false;
  }

  return true;
}

bool LinuxFirewall::clearAllCgroupsV2() {
  if (NetfilterResetAllCgroupsV2() != 0) {
    logger.error() << "Error attempting to clear all cgroupv2 traffic";
    return false;
  }

  return true;
}
