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

WireGuardTunnelLinux::~WireGuardTunnelLinux() {
  MZ_COUNT_DTOR(WireGuardTunnelLinux);
  removeInterfaceIfExists();
  delete m_wgutils;
  delete m_iputils;
  logger.debug() << "WireGuardTunnelLinux destroyed.";
}

IPUtils* WireGuardTunnelLinux::iputils() {
  if (!m_iputils) {
    m_iputils = new IPUtilsLinux(this);
  }
  return m_iputils;
}
