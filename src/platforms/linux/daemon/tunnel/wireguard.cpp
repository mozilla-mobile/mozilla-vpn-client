#include "wireguard.h"

#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("WireGuardTunnelLinux");
}

WireGuardTunnelLinux::WireGuardTunnelLinux(QObject* parent)
    : WireGuardTunnel(parent) {
  MZ_COUNT_CTOR(WireGuardTunnelLinux);
  m_wgutils = new WireguardUtilsLinux(this);
  removeInterfaceIfExists();
  logger.debug() << "WireGuardTunnelLinux created.";
}

IPUtils* WireGuardTunnelLinux::iputils() {
  if (!m_iputils) {
    m_iputils = new IPUtilsLinux(this);
  }
  return m_iputils;
}
