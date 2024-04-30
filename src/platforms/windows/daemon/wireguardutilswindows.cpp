/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wireguardutilswindows.h"

#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2ipdef.h>

#include <QFileInfo>

#include "leakdetector.h"
#include "logger.h"
#include "platforms/windows/windowscommons.h"
#include "windowsdaemon.h"
#include "windowsfirewall.h"

#include "wireguard.h"

#pragma comment(lib, "iphlpapi.lib")

namespace {
Logger logger("WireguardUtilsWindows");

// ID for the Mullvad Split-Tunnel Sublayer Provider
DEFINE_GUID(AAAA, 0xe2c114ee, 0xf32a, 0x4264, 0xa6, 0xcb, 0x3f,
            0xa7, 0x99, 0x63, 0x56, 0xd9);


};  // namespace



/**
* Helper for the WireguardNT api, containing pointers
* to all exposed functions by wireguard.dll
* 
* Open the dll using ::create();
* If the struct is destroyed, the dll is unloaded aswell. 
*/
struct WireGuardAPI {
  WIREGUARD_CREATE_ADAPTER_FUNC* CreateAdapter;
  WIREGUARD_OPEN_ADAPTER_FUNC* OpenAdapter;
  WIREGUARD_CLOSE_ADAPTER_FUNC* CloseAdapter;
  WIREGUARD_GET_ADAPTER_LUID_FUNC* GetAdapterLUID;
  WIREGUARD_GET_RUNNING_DRIVER_VERSION_FUNC*
      GetRunningDriverVersion;
  WIREGUARD_DELETE_DRIVER_FUNC* DeleteDriver;
  WIREGUARD_SET_LOGGER_FUNC* SetLogger;
  WIREGUARD_SET_ADAPTER_LOGGING_FUNC* SetAdapterLogging;
  WIREGUARD_GET_ADAPTER_STATE_FUNC* GetAdapterState;
  WIREGUARD_SET_ADAPTER_STATE_FUNC* SetAdapterState;
  WIREGUARD_GET_CONFIGURATION_FUNC* GetConfiguration;
  WIREGUARD_SET_CONFIGURATION_FUNC* SetConfiguration;
  HMODULE dll; // Handle to DLL

  ~WireGuardAPI(){ 
      if (this->dll){
        FreeLibrary(this->dll);
      }
  }
  /**
  * Opens Wireguard DLL, constructs a  WireGuardAPI object
  * and returns that. 
  * Returns nullptr on failure. 
  */
  static std::unique_ptr<WireGuardAPI> create() {
    auto out = std::make_unique<WireGuardAPI>();
    HMODULE WireGuardDll = LoadLibraryExW(
        L"wireguard.dll", NULL,
        LOAD_LIBRARY_SEARCH_APPLICATION_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!WireGuardDll) return {};

#define X(Name)              \
  ((*(FARPROC*)& out->Name = \
        GetProcAddress(WireGuardDll, "WireGuard##Name")) == NULL)
    if (X(CreateAdapter) || X(OpenAdapter) || X(CloseAdapter) ||
        X(GetAdapterLUID) || X(GetRunningDriverVersion) || X(DeleteDriver) ||
        X(SetLogger) || X(SetAdapterLogging) || X(GetAdapterState) ||
        X(SetAdapterState) || X(GetConfiguration) || X(SetConfiguration))
#undef X
    {
      DWORD LastError = GetLastError();
      FreeLibrary(WireGuardDll);
      SetLastError(LastError);
      return {};
    }
    out->dll = WireGuardDll;
    return out;
  }
};


std::unique_ptr<WireguardUtilsWindows> WireguardUtilsWindows::create(
    QObject* parent) {
  auto wg_nt = WireGuardAPI::create();
  if (!wg_nt) {
    return {};
  }
  // Can't use make_unique here as the Constructor is private :( 
  auto utils = new WireguardUtilsWindows(parent, std::move(wg_nt));
  return std::unique_ptr<WireguardUtilsWindows>(utils);
}

WireguardUtilsWindows::WireguardUtilsWindows(
    QObject* parent, std::unique_ptr<WireGuardAPI> wireguard)
    : WireguardUtils(parent), m_routeMonitor(this), m_wireguard_api(std::move(wireguard)) {
  MZ_COUNT_CTOR(WireguardUtilsWindows);
  logger.debug() << "WireguardUtilsWindows created.";
}

WireguardUtilsWindows::~WireguardUtilsWindows() {
  MZ_COUNT_DTOR(WireguardUtilsWindows);
  logger.debug() << "WireguardUtilsWindows destroyed.";
}

bool WireguardUtilsWindows::interfaceExists() { 
    return m_adapter == NULL; 
}

QList<WireguardUtils::PeerStatus> WireguardUtilsWindows::getPeerStatus() {
  QList<PeerStatus> peerList;
  return peerList;
}

bool WireguardUtilsWindows::addInterface(const InterfaceConfig& config) {
  // TODO: REWRITE
  m_wireguard_api->CreateAdapter(L"W", L"AA", &AAAA );

  auto private_key = QByteArray::fromBase64(config.m_privateKey.toUtf8(),
                                            QByteArray::Base64Encoding);
  WIREGUARD_INTERFACE wgConf = {
      .Flags = WIREGUARD_INTERFACE_HAS_PRIVATE_KEY,
      .ListenPort = 0,  // Choose Randomly
      .PeersCount = 0,
  }; 
  std::copy(std::begin(private_key), std::end(private_key),
            std::begin(wgConf.PrivateKey));


  QStringList addresses;
  for (const IPAddress& ip : config.m_allowedIPAddressRanges) {
    addresses.append(ip.toString());
  }

  QMap<QString, QString> extraConfig;
  extraConfig["Table"] = "off";
  QString configString = config.toWgConf(extraConfig);
  if (configString.isEmpty()) {
    logger.error() << "Failed to create a config file";
    return false;
  }

  // We don't want to pass a peer just yet, that will happen later with
  // a UAPI command in WireguardUtilsWindows::updatePeer(), so truncate
  // the config file to remove the [Peer] section.
  qsizetype peerStart = configString.indexOf("[Peer]", 0, Qt::CaseSensitive);
  if (peerStart >= 0) {
    configString.truncate(peerStart);
  }

  //if (!m_tunnel.start(configString)) {
  //  logger.error() << "Failed to activate the tunnel service";
  //  return false;
 // }

  // Determine the interface LUID
  NET_LUID luid;
  QString ifAlias = interfaceName();
  DWORD result = ConvertInterfaceAliasToLuid((wchar_t*)ifAlias.utf16(), &luid);
  if (result != 0) {
    logger.error() << "Failed to lookup LUID:" << result;
    return false;
  }
  m_luid = luid.Value;
  m_routeMonitor.setLuid(luid.Value);

  // Enable the windows firewall
  NET_IFINDEX ifindex;
  ConvertInterfaceLuidToIndex(&luid, &ifindex);
  WindowsFirewall::instance()->enableKillSwitch(ifindex);

  logger.debug() << "Registration completed";
  return true;
}

bool WireguardUtilsWindows::deleteInterface() {
  // TODO: REWRITE
  WindowsFirewall::instance()->disableKillSwitch();
  //m_tunnel.stop();
  return true;
}

bool WireguardUtilsWindows::updatePeer(const InterfaceConfig& config) {
  // TODO: REWRITE
  QByteArray publicKey =
      QByteArray::fromBase64(qPrintable(config.m_serverPublicKey));

  // Enable the windows firewall for this peer.
  WindowsFirewall::instance()->enablePeerTraffic(config);

  logger.debug() << "Configuring peer" << logger.keys(config.m_serverPublicKey)
                 << "via" << config.m_serverIpv4AddrIn;

  // Update/create the peer config
  QString message;
  QTextStream out(&message);
  out << "set=1\n";
  out << "public_key=" << QString(publicKey.toHex()) << "\n";
  if (!config.m_serverIpv4AddrIn.isNull()) {
    out << "endpoint=" << config.m_serverIpv4AddrIn << ":";
  } else if (!config.m_serverIpv6AddrIn.isNull()) {
    out << "endpoint=[" << config.m_serverIpv6AddrIn << "]:";
  } else {
    logger.warning() << "Failed to create peer with no endpoints";
    return false;
  }
  out << config.m_serverPort << "\n";

  out << "replace_allowed_ips=true\n";
  out << "persistent_keepalive_interval=" << WG_KEEPALIVE_PERIOD << "\n";
  for (const IPAddress& ip : config.m_allowedIPAddressRanges) {
    out << "allowed_ip=" << ip.toString() << "\n";
  }

  // Exclude the server address, except for multihop exit servers.
  if (config.m_hopType != InterfaceConfig::MultiHopExit) {
    m_routeMonitor.addExclusionRoute(IPAddress(config.m_serverIpv4AddrIn));
    m_routeMonitor.addExclusionRoute(IPAddress(config.m_serverIpv6AddrIn));
  }
  // TODO: REWRITE
  //QString reply = m_tunnel.uapiCommand(message);
  //logger.debug() << "DATA:" << reply;
  return true;
}

bool WireguardUtilsWindows::deletePeer(const InterfaceConfig& config) {
    // TODO: REWRITE
  QByteArray publicKey =
      QByteArray::fromBase64(qPrintable(config.m_serverPublicKey));

  // Clear exclustion routes for this peer.
  if (config.m_hopType != InterfaceConfig::MultiHopExit) {
    m_routeMonitor.deleteExclusionRoute(IPAddress(config.m_serverIpv4AddrIn));
    m_routeMonitor.deleteExclusionRoute(IPAddress(config.m_serverIpv6AddrIn));
  }

  // Disable the windows firewall for this peer.
  WindowsFirewall::instance()->disablePeerTraffic(config.m_serverPublicKey);

  QString message;
  QTextStream out(&message);
  out << "set=1\n";
  out << "public_key=" << QString(publicKey.toHex()) << "\n";
  out << "remove=true\n";

  //QString reply = m_tunnel.uapiCommand(message);
  //logger.debug() << "DATA:" << reply;
  return true;
}

void WireguardUtilsWindows::buildMibForwardRow(const IPAddress& prefix,
                                               void* row) {
  MIB_IPFORWARD_ROW2* entry = (MIB_IPFORWARD_ROW2*)row;
  InitializeIpForwardEntry(entry);

  // Populate the next hop
  if (prefix.type() == QAbstractSocket::IPv6Protocol) {
    InetPtonA(AF_INET6, qPrintable(prefix.address().toString()),
              &entry->DestinationPrefix.Prefix.Ipv6.sin6_addr);
    entry->DestinationPrefix.Prefix.Ipv6.sin6_family = AF_INET6;
    entry->DestinationPrefix.PrefixLength = prefix.prefixLength();
  } else {
    InetPtonA(AF_INET, qPrintable(prefix.address().toString()),
              &entry->DestinationPrefix.Prefix.Ipv4.sin_addr);
    entry->DestinationPrefix.Prefix.Ipv4.sin_family = AF_INET;
    entry->DestinationPrefix.PrefixLength = prefix.prefixLength();
  }
  entry->InterfaceLuid.Value = m_luid;
  entry->NextHop.si_family = entry->DestinationPrefix.Prefix.si_family;

  // Set the rest of the flags for a static route.
  entry->ValidLifetime = 0xffffffff;
  entry->PreferredLifetime = 0xffffffff;
  entry->Metric = 0;
  entry->Protocol = MIB_IPPROTO_NETMGMT;
  entry->Loopback = false;
  entry->AutoconfigureAddress = false;
  entry->Publish = false;
  entry->Immortal = false;
  entry->Age = 0;
}

bool WireguardUtilsWindows::updateRoutePrefix(const IPAddress& prefix) {
  MIB_IPFORWARD_ROW2 entry;
  buildMibForwardRow(prefix, &entry);

  // Install the route
  DWORD result = CreateIpForwardEntry2(&entry);
  if (result == ERROR_OBJECT_ALREADY_EXISTS) {
    return true;
  }
  if (result != NO_ERROR) {
    logger.error() << "Failed to create route to"
                   << logger.sensitive(prefix.toString())
                   << "result:" << result;
  }
  return result == NO_ERROR;
}

bool WireguardUtilsWindows::deleteRoutePrefix(const IPAddress& prefix) {
  MIB_IPFORWARD_ROW2 entry;
  buildMibForwardRow(prefix, &entry);

  // Install the route
  DWORD result = DeleteIpForwardEntry2(&entry);
  if (result == ERROR_NOT_FOUND) {
    return true;
  }
  if (result != NO_ERROR) {
    logger.error() << "Failed to delete route to"
                   << logger.sensitive(prefix.toString())
                   << "result:" << result;
  }
  return result == NO_ERROR;
}

bool WireguardUtilsWindows::addExclusionRoute(const IPAddress& prefix) {
  return m_routeMonitor.addExclusionRoute(prefix);
}

bool WireguardUtilsWindows::deleteExclusionRoute(const IPAddress& prefix) {
  return m_routeMonitor.deleteExclusionRoute(prefix);
}
