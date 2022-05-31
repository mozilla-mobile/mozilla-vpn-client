/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosnetworkwatcher.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"

#import <CoreWLAN/CoreWLAN.h>
#import <Network/Network.h>

namespace {
Logger logger(LOG_MACOS, "MacOSNetworkWatcher");
dispatch_queue_t s_queue = dispatch_queue_create("VPNNetwork.queue", DISPATCH_QUEUE_SERIAL);

}

@interface MacOSNetworkWatcherDelegate : NSObject <CWEventDelegate> {
  MacOSNetworkWatcher* m_watcher;
}
@end

@implementation MacOSNetworkWatcherDelegate

- (id)initWithObject:(MacOSNetworkWatcher*)watcher {
  self = [super init];
  if (self) {
    m_watcher = watcher;
  }
  return self;
}

- (void)bssidDidChangeForWiFiInterfaceWithName:(NSString*)interfaceName {
  logger.debug() << "BSSID changed!" << QString::fromNSString(interfaceName);

  if (m_watcher) {
    m_watcher->checkInterface();
  }
}

@end

MacOSNetworkWatcher::MacOSNetworkWatcher(QObject* parent) : NetworkWatcherImpl(parent) {
  MVPN_COUNT_CTOR(MacOSNetworkWatcher);
}

MacOSNetworkWatcher::~MacOSNetworkWatcher() {
  MVPN_COUNT_DTOR(MacOSNetworkWatcher);
  if (m_delegate) {
    CWWiFiClient* client = CWWiFiClient.sharedWiFiClient;
    if (!client) {
      logger.debug() << "Unable to retrieve the CWWiFiClient shared instance";
      return;
    }

    [client stopMonitoringAllEventsAndReturnError:nullptr];
    [static_cast<MacOSNetworkWatcherDelegate*>(m_delegate) dealloc];
    m_delegate = nullptr;
  }
}

void MacOSNetworkWatcher::initialize() {
  connect(MozillaVPN::instance()->controller(), &Controller::stateChanged, this,
          &MacOSNetworkWatcher::controllerStateChanged);
  auto mon = nw_path_monitor_create();
  nw_path_monitor_set_queue(mon, s_queue);
  nw_path_monitor_set_update_handler(mon, ^(nw_path_t _Nonnull path) {
    mCurrentDefaultTransport = toTransportType(path);
  });
  nw_path_monitor_start(mon);
  m_networkMonitor = &mon;
}

void MacOSNetworkWatcher::start() {
  NetworkWatcherImpl::start();

  checkInterface();

  if (m_delegate) {
    logger.debug() << "Delegate already registered";
    return;
  }

  CWWiFiClient* client = CWWiFiClient.sharedWiFiClient;
  if (!client) {
    logger.error() << "Unable to retrieve the CWWiFiClient shared instance";
    return;
  }

  logger.debug() << "Registering delegate";
  m_delegate = [[MacOSNetworkWatcherDelegate alloc] initWithObject:this];
  [client setDelegate:static_cast<MacOSNetworkWatcherDelegate*>(m_delegate)];
  [client startMonitoringEventWithType:CWEventTypeBSSIDDidChange error:nullptr];
}

void MacOSNetworkWatcher::checkInterface() {
  logger.debug() << "Checking interface";

  if (!isActive()) {
    logger.debug() << "Feature disabled";
    return;
  }

  CWWiFiClient* client = CWWiFiClient.sharedWiFiClient;
  if (!client) {
    logger.debug() << "Unable to retrieve the CWWiFiClient shared instance";
    return;
  }

  CWInterface* interface = [client interface];
  if (!interface) {
    logger.debug() << "No default wifi interface";
    return;
  }

  if (![interface powerOn]) {
    logger.debug() << "The interface is off";
    return;
  }

  NSString* ssidNS = [interface ssid];
  if (!ssidNS) {
    logger.debug() << "WiFi is not in used";
    return;
  }

  QString ssid = QString::fromNSString(ssidNS);
  if (ssid.isEmpty()) {
    logger.debug() << "WiFi doesn't have a valid SSID";
    return;
  }

  CWSecurity security = [interface security];
  if (security == kCWSecurityNone || security == kCWSecurityWEP) {
    logger.debug() << "Unsecured network found!";
    emit unsecuredNetwork(ssid, ssid);
    return;
  }

  logger.debug() << "Secure WiFi interface";
}

NetworkWatcherImpl::TransportType MacOSNetworkWatcher::getTransportType() {
  if (MozillaVPN::instance()->controller()->state() != Controller::StateOn) {
    // If we're not in stateON, the result from the Observer is fine, as
    // the default-route-transport  is not the vpn-tunnel
    return mCurrentDefaultTransport;
  }
  if (m_observableConnection != nil) {
    return mVPNTunnelTransport;
  }
  // If we don't have an open tunnel-observer, mVPNTunnelTransport is probably wrong.
  return NetworkWatcherImpl::TransportType_Unknown;
}

void MacOSNetworkWatcher::controllerStateChanged() {
  if (MozillaVPN::instance()->controller()->state() != Controller::StateOn) {
    if (m_observableConnection != nil) {
      nw_connection_cancel(m_observableConnection);
      m_observableConnection = nil;
    }
    return;
  }
  // When connected the current default route is the VPN (ofc)
  // therefore the monitor will report transport::other.
  // We need to open up a theoretical socket to our entry-node, to find out
  // what connection the route it's using
  auto vpn = MozillaVPN::instance();
  // When multihop is used, we need to connect to the entry server,
  // otherwise the exit server is the target
  auto key = vpn->multihop() ? vpn->entryServerPublicKey() : vpn->serverPublicKey();
  auto serverlist = vpn->multihop() ? vpn->entryServers() : vpn->exitServers();
  auto index = serverlist.indexOf(key);
  // No such server
  if (index == -1) {
    logger.error() << "Unable to find the right server for " << key;
    return;
  }
  auto server = serverlist.at(index);
  auto str = server.ipv4AddrIn().toStdString();
  auto endpoint = nw_endpoint_create_host(str.c_str(), "80");
  auto params = nw_parameters_create_secure_udp(NW_PARAMETERS_DISABLE_PROTOCOL,
                                                NW_PARAMETERS_DEFAULT_CONFIGURATION);
  auto con = nw_connection_create(endpoint, params);
  nw_connection_set_queue(con, s_queue);
  nw_connection_set_path_changed_handler(con, ^(nw_path_t _Nonnull path) {
    logger.debug() << "VPN connection path changed";
    nw_path_enumerate_interfaces(path, ^bool(nw_interface_t _Nonnull interface) {
      auto name = nw_interface_get_name(interface);
      logger.debug() << "Using Interface: " << name;
      return true;
    });
    mVPNTunnelTransport = toTransportType(path);
  });
  nw_connection_start(con);
  logger.info() << "Opened udp to:" << server.hostname();
  m_observableConnection = con;
}

NetworkWatcherImpl::TransportType MacOSNetworkWatcher::toTransportType(nw_path_t path) {
  if (path == nil) {
    return NetworkWatcherImpl::TransportType_Unknown;
  }
  auto status = nw_path_get_status(path);
  if (status != nw_path_status_satisfied && status != nw_path_status_satisfiable) {
    // We're offline.
    return NetworkWatcherImpl::TransportType_None;
  }
  if (nw_path_uses_interface_type(path, nw_interface_type_wifi)) {
    return NetworkWatcherImpl::TransportType_WiFi;
  }
  if (nw_path_uses_interface_type(path, nw_interface_type_wired)) {
    return NetworkWatcherImpl::TransportType_Ethernet;
  }
  if (nw_path_uses_interface_type(path, nw_interface_type_cellular)) {
    return NetworkWatcherImpl::TransportType_Cellular;
  }
  if (nw_path_uses_interface_type(path, nw_interface_type_other)) {
    return NetworkWatcherImpl::TransportType_Other;
  }
  if (nw_path_uses_interface_type(path, nw_interface_type_loopback)) {
    return NetworkWatcherImpl::TransportType_Other;
  }
  return NetworkWatcherImpl::TransportType_Unknown;
}
