/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wireguardutilswindows.h"

#include <Dbghelp.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2ipdef.h>

#include <QFileInfo>
#include <iostream>

#include "leakdetector.h"
#include "logger.h"
#include "platforms/windows/windowscommons.h"
#include "platforms/windows/windowsutils.h"
#include "windowsdaemon.h"
#include "windowsfirewall.h"
#include "windowsroutemonitor.h"
#include "wireguard.h"
DEFINE_ENUM_FLAG_OPERATORS(WIREGUARD_PEER_FLAG)
DEFINE_ENUM_FLAG_OPERATORS(WIREGUARD_INTERFACE_FLAG)

#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "iphlpapi.lib")

namespace {
Logger logger("WireguardUtilsWindows");

GUID ADAPTER_GUID = {0xf64063ab,
                     0xbfee,
                     0x4881,
                     {0xbf, 0x79, 0x36, 0x6e, 0x4c, 0xc7, 0xba, 0x75}};

Logger nt_logger("WireGuardNT");

static void CALLBACK WireGuardLogger(_In_ WIREGUARD_LOGGER_LEVEL Level,
                                     _In_ DWORD64 Timestamp,
                                     _In_z_ const WCHAR* LogLine) {
  Q_UNUSED(Timestamp);
  auto msg = QString::fromWCharArray(LogLine);
  switch (Level) {
    case WIREGUARD_LOG_INFO:
      nt_logger.info() << msg;
      break;
    case WIREGUARD_LOG_WARN:
      nt_logger.warning() << msg;
      break;
    case WIREGUARD_LOG_ERR:
      nt_logger.error() << msg;
      break;
    default:
      return;
  }
}

/**
 * @brief Assigns an ipv4 address to a network device with a given LUID
 *
 * @param luid - LUID of the Adapter
 * @param address - Address and NetMask of the Adapter
 * @return ulong - nteContext - Call DeleteIPAddress(nteContext) to remove the
 * assignment.
 */
ulong setIPv4AddressAndMask(NET_LUID luid, const IPAddress address) {
  ULONG nteContext = 0;
  ULONG nteInstance = 0;
  NET_IFINDEX ifIndex;
  if (ConvertInterfaceLuidToIndex(&luid, &ifIndex) != NO_ERROR) {
    return 0;
  }
  IN_ADDR ipAddrBinary, subnetMaskBinary;
  if (InetPtonA(AF_INET, qPrintable(address.address().toString()),
                &ipAddrBinary) != 1) {
    return 0;
  }
  if (InetPtonA(AF_INET, qPrintable(address.netmask().toString()),
                &subnetMaskBinary) != 1) {
    return 0;
  }
  // Add IP address and subnet mask
  DWORD dwResult =
      AddIPAddress(ipAddrBinary.S_un.S_addr, subnetMaskBinary.S_un.S_addr,
                   ifIndex, &nteContext, &nteInstance);
  if (dwResult != NO_ERROR) {
    WindowsUtils::windowsLog("WELP, failed to add ip address to adapter");
    return 0;
  }
  return nteContext;
}
/**
 * @brief
 *
 * @param luid - LUID of the Adapter
 * @param ipAddress - Address and NetMask of the Adapter
 * @return bool - If the assignment was successful
 */
bool setIPv6AddressAndMask(NET_LUID luid, const IPAddress ipAddress) {
  MIB_UNICASTIPADDRESS_ROW row;

  NET_IFINDEX ifIndex;
  if (ConvertInterfaceLuidToIndex(&luid, &ifIndex) != NO_ERROR) {
    logger.error()
        << "Failed to assign ivp6: Cannot Find Interface for this LUID";
    return false;
  }
  SOCKADDR_IN6 sockaddr = {};
  sockaddr.sin6_family = AF_INET6;
  if (InetPtonA(AF_INET6, qPrintable(ipAddress.address().toString()),
                &sockaddr.sin6_addr) != 1) {
    logger.error() << "Failed to assign ivp6: Cannot Parse IPv6 Address "
                   << ipAddress.address().toString();
    return false;
  }

  InitializeUnicastIpAddressEntry(&row);
  row.Address.Ipv6.sin6_family = AF_INET6;
  row.Address.Ipv6 = sockaddr;
  row.InterfaceLuid = luid;

  // Calculate prefix length from subnet mask
  unsigned int prefixLength = ipAddress.prefixLength();
  row.OnLinkPrefixLength = prefixLength;

  DWORD dwResult = CreateUnicastIpAddressEntry(&row);
  if (dwResult != NO_ERROR) {
    logger.error() << "Failed to assign ivp6: CreateUnicastIpAddressEntry "
                      "failed with error : "
                   << dwResult;
    return false;
  }

  return true;
}

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
  WIREGUARD_GET_RUNNING_DRIVER_VERSION_FUNC* GetRunningDriverVersion;
  WIREGUARD_DELETE_DRIVER_FUNC* DeleteDriver;
  WIREGUARD_SET_LOGGER_FUNC* SetLogger;
  WIREGUARD_SET_ADAPTER_LOGGING_FUNC* SetAdapterLogging;
  WIREGUARD_GET_ADAPTER_STATE_FUNC* GetAdapterState;
  WIREGUARD_SET_ADAPTER_STATE_FUNC* SetAdapterState;
  WIREGUARD_GET_CONFIGURATION_FUNC* GetConfiguration;
  WIREGUARD_SET_CONFIGURATION_FUNC* SetConfiguration;
  HMODULE dll;  // Handle to DLL
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
    if (WireGuardDll == nullptr) {
      WindowsUtils::windowsLog("Failed to open wireguard.dll");
      return {};
    }

    WindowsUtils::windowsLog("Wireguard DLL FOUND!");

    auto const getFunc = [WireGuardDll](LPCSTR lpProcName, auto* ref) -> bool {
      auto func = GetProcAddress(WireGuardDll, lpProcName);
      if (func == NULL) {
        WindowsUtils::windowsLog("Failed to get ");
        WindowsUtils::windowsLog(QString::fromLocal8Bit(lpProcName));
        return false;
      }
      static_assert(sizeof(ref) == sizeof(FARPROC));
      std::memcpy(ref, &func, sizeof(FARPROC));
      return true;
    };
    std::array ok = {
        getFunc("WireGuardCreateAdapter", &out->CreateAdapter),
        getFunc("WireGuardOpenAdapter", &out->OpenAdapter),
        getFunc("WireGuardCloseAdapter", &out->CloseAdapter),
        getFunc("WireGuardGetAdapterLUID", &out->GetAdapterLUID),
        getFunc("WireGuardGetRunningDriverVersion",
                &out->GetRunningDriverVersion),
        getFunc("WireGuardDeleteDriver", &out->DeleteDriver),
        getFunc("WireGuardSetLogger", &out->SetLogger),
        getFunc("WireGuardSetAdapterLogging", &out->SetAdapterLogging),
        getFunc("WireGuardGetAdapterState", &out->GetAdapterState),
        getFunc("WireGuardSetAdapterState", &out->SetAdapterState),
        getFunc("WireGuardGetConfiguration", &out->GetConfiguration),
        getFunc("WireGuardSetConfiguration", &out->SetConfiguration)};
    if (!std::ranges::all_of(ok, [](bool v) { return v; })) {
      return {};
    };
    WindowsUtils::windowsLog("OPENED wireguard.dll");
    out->SetLogger(WireGuardLogger);
    out->dll = WireGuardDll;
    return out;
  }
  ~WireGuardAPI() {
    if (this->dll) {
      FreeLibrary(this->dll);
    }
  }
};

std::unique_ptr<WireguardUtilsWindows> WireguardUtilsWindows::create(
    QObject* parent) {
  auto wg_nt = WireGuardAPI::create();
  if (!wg_nt) {
    WindowsUtils::windowsLog("Failed to get a wireguard.dll");
    return {};
  }
  // Can't use make_unique here as the Constructor is private :(
  auto utils = new WireguardUtilsWindows(parent, std::move(wg_nt));
  return std::unique_ptr<WireguardUtilsWindows>(utils);
}

WireguardUtilsWindows::WireguardUtilsWindows(
    QObject* parent, std::unique_ptr<WireGuardAPI> wireguard)
    : WireguardUtils(parent), m_wireguard_api(std::move(wireguard)) {
  MZ_COUNT_CTOR(WireguardUtilsWindows);
  logger.debug() << "WireguardUtilsWindows created.";
}

WireguardUtilsWindows::~WireguardUtilsWindows() {
  MZ_COUNT_DTOR(WireguardUtilsWindows);
  logger.debug() << "WireguardUtilsWindows destroyed.";
  DeleteIPAddress(m_deviceIpv4_Handle);
  if (m_adapter) {
    m_wireguard_api->SetAdapterState(m_adapter, WIREGUARD_ADAPTER_STATE_DOWN);
    m_wireguard_api->CloseAdapter(m_adapter);
  }
}

bool WireguardUtilsWindows::interfaceExists() { return m_adapter != NULL; }

/**
 * Creates a Wireguard Adapter with that Private Key
 * -> Enables the Initial Killswitch for that Adapter
 * -> Enables the Route Monitor for that Adapter
 * Returns true on success.
 */
bool WireguardUtilsWindows::addInterface(const InterfaceConfig& config) {
  // Create the Adapter and Cleanup fallbacks in case of failure.
  WIREGUARD_ADAPTER_HANDLE wireguard_adapter = m_wireguard_api->CreateAdapter(
      (const wchar_t*)interfaceName().utf16(), L"Mozilla", &ADAPTER_GUID);
  if (wireguard_adapter == NULL) {
    logger.error() << "Failed creating Wireguard Adapter";
    return false;
  }
  auto cleanupDeviceOnFailure = qScopeGuard([wireguard_adapter, this]() {
    logger.error() << "Failure during Adapter Creation, Closing Device";
    m_wireguard_api->CloseAdapter(wireguard_adapter);
  });

  // Set the Private Key of the Device.
  WIREGUARD_INTERFACE wgConf = {
      .Flags = WIREGUARD_INTERFACE_HAS_PRIVATE_KEY,
      .ListenPort = 0,  // Choose Randomly
      .PeersCount = 0,  // that will happen later with updatePeer()
  };
  auto private_key = QByteArray::fromBase64(config.m_privateKey.toUtf8(),
                                            QByteArray::Base64Encoding);
  std::copy(std::begin(private_key), std::end(private_key),
            std::begin(wgConf.PrivateKey));

  if (!m_wireguard_api->SetConfiguration(wireguard_adapter, &wgConf,
                                         sizeof(wgConf))) {
    logger.error() << "Failed setting Wireguard Adapter Config";
    return false;
  }
  if (!m_wireguard_api->SetAdapterState(wireguard_adapter,
                                        WIREGUARD_ADAPTER_STATE_UP)) {
    return false;
  }
#ifdef MZ_DEBUG
  // Wireguard does print things like "Receiving handshake response from peer 2
  // (178.255.149.140:2730)" as we probably do not want to record which ip the
  // users connect to, let's make it debug only.
  m_wireguard_api->SetAdapterLogging(wireguard_adapter,
                                     WIREGUARD_ADAPTER_LOG_ON);
#endif

  // Determine the interface LUID
  NET_LUID luid;
  m_wireguard_api->GetAdapterLUID(wireguard_adapter, &luid);
  m_luid = luid.Value;

  m_routeMonitor = new WindowsRouteMonitor(luid.Value, this);
  auto destroyRouteMonitorOnFailure = qScopeGuard([this]() {
    delete m_routeMonitor;
    m_routeMonitor = nullptr;
  });

  // Set the Adapters Address:
  m_deviceIpv4_Handle =
      setIPv4AddressAndMask(luid, IPAddress(config.m_deviceIpv4Address));
  if (m_deviceIpv4_Handle == 0) {
    logger.error() << "Failed setIPv4AddressAndMask";
    return false;
  }
  if (!setIPv6AddressAndMask(luid, IPAddress(config.m_deviceIpv6Address))) {
    logger.error() << "Failed setIPv6AddressAndMask";
    return false;
  };

  // Enable the windows firewall
  NET_IFINDEX ifindex;
  ConvertInterfaceLuidToIndex(&luid, &ifindex);
  if (!WindowsFirewall::instance()->enableInterface(ifindex)) {
    logger.error() << "Failed enabling Killswitch";
    WindowsFirewall::instance()->disableKillSwitch();
    return false;
  };
  cleanupDeviceOnFailure.dismiss();
  destroyRouteMonitorOnFailure.dismiss();
  m_adapter = wireguard_adapter;
  return true;
}

/**
 * Turns down the Adapter and Deletes the Interface
 */
bool WireguardUtilsWindows::deleteInterface() {
  if (!m_adapter) {
    return false;
  }
  if (m_routeMonitor) {
    delete m_routeMonitor;
    m_routeMonitor = nullptr;
  }
  if (m_deviceIpv4_Handle != 0) {
    DeleteIPAddress(m_deviceIpv4_Handle);
  }
  WindowsFirewall::instance()->disableKillSwitch();
  m_wireguard_api->SetAdapterState(m_adapter, WIREGUARD_ADAPTER_STATE_DOWN);
  m_wireguard_api->CloseAdapter(m_adapter);
  m_adapter = NULL;
  return true;
}

bool WireguardUtilsWindows::updatePeer(const InterfaceConfig& config) {
  // Enable the windows firewall for this peer.
  if (!WindowsFirewall::instance()->enablePeerTraffic(config)) {
    return false;
  };

#pragma pack(push, 1)
  struct WireGuardNTConfig {
    WIREGUARD_INTERFACE interface;
    WIREGUARD_PEER peer;
    std::array<WIREGUARD_ALLOWED_IP, 2> allowedIP;
  };
#pragma pack(pop)
  auto wgnt_conf = WireGuardNTConfig{.interface{.PeersCount = 1}};
  wgnt_conf.peer.PersistentKeepalive = WG_KEEPALIVE_PERIOD;

  // TODO: We currently assume an ipv4 reachable endpoint
  // we need to make sure this is the right decision.
  wgnt_conf.peer.Endpoint.si_family = AF_INET;
  wgnt_conf.peer.Endpoint.Ipv4.sin_family = AF_INET;
  wgnt_conf.peer.Endpoint.Ipv4.sin_port = config.m_serverPort;

  if (!inet_pton(AF_INET, qPrintable(config.m_serverIpv4AddrIn),
                 &wgnt_conf.peer.Endpoint.Ipv4.sin_addr)) {
    logger.error() << "Failed to parse m_serverIpv4AddrIn";
    return false;
  }

  auto peer_public_key = QByteArray::fromBase64(
      config.m_serverPublicKey.toUtf8(), QByteArray::Base64Encoding);
  std::copy(std::begin(peer_public_key), std::end(peer_public_key),
            std::begin(wgnt_conf.peer.PublicKey));

  wgnt_conf.peer.Flags =
      WIREGUARD_PEER_HAS_PUBLIC_KEY | WIREGUARD_PEER_HAS_PERSISTENT_KEEPALIVE |
      WIREGUARD_PEER_HAS_ENDPOINT | WIREGUARD_PEER_REPLACE_ALLOWED_IPS;

  logger.debug() << "Configuring peer" << logger.keys(config.m_serverPublicKey)
                 << "via" << config.m_serverIpv4AddrIn;

  // Everything that has reached the wireguard adapter should be accepted
  wgnt_conf.peer.AllowedIPsCount = 2;
  wgnt_conf.allowedIP[0] = WIREGUARD_ALLOWED_IP{.AddressFamily = AF_INET};
  wgnt_conf.allowedIP[1] = WIREGUARD_ALLOWED_IP{.AddressFamily = AF_INET6};

  if (!m_wireguard_api->SetConfiguration(m_adapter, &wgnt_conf.interface,
                                         sizeof(wgnt_conf))) {
    logger.error() << "Failed setting Wireguard Adapter Config";
    return false;
  }
  // Exclude the server address, except for multihop exit servers.
  if (m_routeMonitor && (config.m_hopType != InterfaceConfig::MultiHopExit)) {
    m_routeMonitor->addExclusionRoute(IPAddress(config.m_serverIpv4AddrIn));
    m_routeMonitor->addExclusionRoute(IPAddress(config.m_serverIpv6AddrIn));
  }
  return true;
}

QList<WireguardUtils::PeerStatus> WireguardUtilsWindows::getPeerStatus() {
  if (!m_adapter) {
    return {};
  }
  DWORD bufferSize = 1024;
  auto buffer = std::array<uint8_t, 2048>{};

  bool ok = m_wireguard_api->GetConfiguration(
      m_adapter, (WIREGUARD_INTERFACE*)&buffer, &bufferSize);
  if (!ok) {
    return {};
  }
  QList<PeerStatus> peerList;
  /**
   * The data we get from GetConfiguration is as follows:
   * WIREGUARD_INTERFACE -> has N .peerCounts
   * WIREGUARD_PEER peer1 -> i.e has 1 allowed_IP
   * ALLOWED_IP peer1_allowedIp_1
   * WIREGUARD_PEER peer2
   * ALLOWED_IP peer2_allowedIp_1
   * ALLOWED_IP peer2_allowedIp_2
   * ....
   *
   */
  auto iFaceConfig = (WIREGUARD_INTERFACE*)&buffer.at(0);
  int peerCount = iFaceConfig->PeersCount;
  int index = 0 + sizeof(WIREGUARD_INTERFACE);
  do {
    if (index > (int)bufferSize) {
      // SOMETHING IS OFF!
      Q_ASSERT(false);
      return {};
    }
    // Pray this is a peer.
    auto peer = (WIREGUARD_PEER*)&buffer.at(index);
    if (peer->PersistentKeepalive == WG_KEEPALIVE_PERIOD &&
        (peer->Flags & WIREGUARD_PEER_HAS_PUBLIC_KEY)) {
      // The peer seems to be okay so
      // Fill in Data
      auto b64_key =
          QByteArray::fromRawData((const char*)peer->PublicKey, 32).toBase64();
      auto status = PeerStatus{QString(b64_key)};
      status.m_handshake = peer->LastHandshake;
      status.m_rxBytes = peer->RxBytes;
      status.m_txBytes = peer->TxBytes;
      peerList.append(status);
    }

    // Calculate the next index.
    index = index + sizeof(WIREGUARD_PEER) +
            sizeof(WIREGUARD_ALLOWED_IP) * peer->AllowedIPsCount;
  } while (peerList.count() < peerCount);
  return peerList;
}

/**
 * Removes a Peer matching the InterfaceConfig's Public Key
 */
bool WireguardUtilsWindows::deletePeer(const InterfaceConfig& config) {
  const auto currentPeers = getPeerStatus().count();
#pragma pack(push, 1)
  struct WireGuardNTConfig {
    WIREGUARD_INTERFACE interface;
    WIREGUARD_PEER peer;
  };
#pragma pack(pop)

  auto wgnt_conf = WireGuardNTConfig{
      .interface{.PeersCount = 1},
      .peer{
          .Flags = WIREGUARD_PEER_HAS_PUBLIC_KEY | WIREGUARD_PEER_REMOVE,
          .AllowedIPsCount = 0,
      }};
  const auto peer_public_key = QByteArray::fromBase64(
      config.m_serverPublicKey.toUtf8(), QByteArray::Base64Encoding);
  std::copy(std::begin(peer_public_key), std::end(peer_public_key),
            std::begin(wgnt_conf.peer.PublicKey));

  if (!m_wireguard_api->SetConfiguration(m_adapter, &wgnt_conf.interface,
                                         sizeof(wgnt_conf))) {
    logger.error() << "Failed deletePeer";
    return false;
  }
  Q_ASSERT(getPeerStatus().count() < currentPeers);
  // Clear exclustion routes for this peer.
  if (m_routeMonitor && (config.m_hopType != InterfaceConfig::MultiHopExit)) {
    m_routeMonitor->deleteExclusionRoute(IPAddress(config.m_serverIpv4AddrIn));
    m_routeMonitor->deleteExclusionRoute(IPAddress(config.m_serverIpv6AddrIn));
  }
  // Disable the windows firewall for this peer.
  WindowsFirewall::instance()->disablePeerTraffic(config.m_serverPublicKey);
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
  if (m_routeMonitor && (prefix.prefixLength() == 0)) {
    // If we are setting up a default route, instruct the route monitor to
    // capture traffic to all non-excluded destinations
    m_routeMonitor->setDetaultRouteCapture(true);
  }

  // Build the route
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
  if (m_routeMonitor && (prefix.prefixLength() == 0)) {
    // Deactivate the route capture feature.
    m_routeMonitor->setDetaultRouteCapture(false);
  }

  // Build the route
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

bool WireguardUtilsWindows::excludeLocalNetworks(
    const QList<IPAddress>& addresses) {
  // If the interface isn't up then something went horribly wrong.
  Q_ASSERT(m_routeMonitor);

  // For each destination - attempt to exclude it from the VPN tunnel.
  bool result = true;
  for (const IPAddress& prefix : addresses) {
    if (!m_routeMonitor->addExclusionRoute(prefix)) {
      result = false;
    }
  }

  // Permit LAN traffic through the firewall.
  if (!WindowsFirewall::instance()->enableLanBypass(addresses)) {
    result = false;
  }

  return result;
}
