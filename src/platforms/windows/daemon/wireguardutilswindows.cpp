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

// TODO: Change this 
GUID ExampleGuid = {0xdeadc001,
                    0xbeef,
                    0xbabe,
                    {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef}};

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

/**
* Creates a Wireguard Adapter with that Private Key
* -> Enables the Initial Killswitch for that Adapter
* -> Enables the Route Monitor for that Adapter
* Returns true on success. 
*/
bool WireguardUtilsWindows::addInterface(const InterfaceConfig& config) {
    // TODO: What should those values be?
  WIREGUARD_ADAPTER_HANDLE wireguard_adapter =
        m_wireguard_api->CreateAdapter(L"W", L"AA", &ExampleGuid);
  if (wireguard_adapter == NULL) {
    logger.error() << "Failed creating Wireguard Adapter";
    return false;
  }
  auto cleanupDeviceOnFailure =
      qScopeGuard([wireguard_adapter, this]() { 
      logger.error() << "Failure during Adapter Creation, Closing Device";
      m_wireguard_api->CloseAdapter(wireguard_adapter); 
  });

  auto private_key = QByteArray::fromBase64(config.m_privateKey.toUtf8(),
                                            QByteArray::Base64Encoding);

  WIREGUARD_INTERFACE wgConf = {
      .Flags = WIREGUARD_INTERFACE_HAS_PRIVATE_KEY,
      .ListenPort = 0,  // Choose Randomly
      .PeersCount = 0,  // that will happen later with updatePeer()
  };
  std::copy(std::begin(private_key), std::end(private_key),
            std::begin(wgConf.PrivateKey));

  if (!m_wireguard_api->SetConfiguration(wireguard_adapter, &wgConf, sizeof(wgConf))) {
    logger.error() << "Failed setting Wireguard Adapter Config";
    return false;
  }
  if (!m_wireguard_api->SetAdapterState(wireguard_adapter, WIREGUARD_ADAPTER_STATE_UP)){
    return false;
}

  // Determine the interface LUID
  NET_LUID luid;
  m_wireguard_api->GetAdapterLUID(wireguard_adapter, &luid);
  m_luid = luid.Value;
  m_routeMonitor.setLuid(luid.Value);
  auto resetLUIDOnFailure =
      qScopeGuard([this]() { 
          m_routeMonitor.setLuid(0); 
          m_luid = 0;
  });

  // Enable the windows firewall
  NET_IFINDEX ifindex;
  ConvertInterfaceLuidToIndex(&luid, &ifindex);
  if (!WindowsFirewall::instance()->enableKillSwitch(ifindex)) {
    logger.error() << "Failed enabling Killswitch";
    WindowsFirewall::instance()->disableKillSwitch();
    return false;
  };
  cleanupDeviceOnFailure.dismiss();
  resetLUIDOnFailure.dismiss();
  m_adapter = wireguard_adapter; 
  return true;
}

/**
* Turns down the Adapter and Deletes the Interface
*/
bool WireguardUtilsWindows::deleteInterface() {
  if (!m_adapter) {
    return;
  }
  WindowsFirewall::instance()->disableKillSwitch();
  m_wireguard_api->SetAdapterState(m_adapter, WIREGUARD_ADAPTER_STATE_DOWN); 
  m_wireguard_api->CloseAdapter(m_adapter); 
  m_adapter = NULL;
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
