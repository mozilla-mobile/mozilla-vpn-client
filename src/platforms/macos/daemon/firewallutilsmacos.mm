/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "firewallutilsmacos.h"

#include "leakdetector.h"
#include "logger.h"
#include "macossplittunnelloader.h"

#import <Foundation/Foundation.h>
#import <NetworkExtension/NetworkExtension.h>
#import <SystemExtensions/SystemExtensions.h>

namespace {
Logger logger("FirewallUtilsMacos");
}  // namespace

FirewallUtilsMacos::FirewallUtilsMacos(Daemon* daemon) : FirewallUtils(daemon) {
  MZ_COUNT_CTOR(FirewallUtilsMacos);
  logger.debug() << "FirewallUtilsMacos created.";
  m_daemon = daemon;

  // Create the Obj-C loader class.
  MacosSplitTunnelLoader* loader = [MacosSplitTunnelLoader new];
  [loader retain];
  m_loader = loader;

  // Create a request to install the system extension.
  dispatch_queue_t queue =
      dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
  OSSystemExtensionRequest* req =
      [OSSystemExtensionRequest activationRequestForExtension: loader.identifier
                                                        queue: queue];
  req.delegate = loader;

  // Start the request
  logger.debug() << "activation request started:" << req.identifier;
  [[OSSystemExtensionManager sharedManager] submitRequest: req];
}

FirewallUtilsMacos::~FirewallUtilsMacos() {
  MZ_COUNT_DTOR(FirewallUtilsMacos);
  logger.debug() << "FirewallUtilsMacos destroyed.";
  [static_cast<MacosSplitTunnelLoader*>(m_loader) release];
}

bool FirewallUtilsMacos::enable(const InterfaceConfig& config) {
  logger.debug() << "MacOS Firewall enabled";
  Q_ASSERT(m_session == nullptr);

  // Create a new tunnel provider session.
  auto loader = static_cast<MacosSplitTunnelLoader*>(m_loader);
  if ((loader.manager == nil) || !loader.manager.enabled) {
    // Split tunnelling is not supported.
    return true;
  }

  // Get a session and start it.
  NSError* error = nil;
  NETunnelProviderSession* session =
      static_cast<NETunnelProviderSession*>(loader.manager.connection);
  // Start the split tunnel proxy.
  BOOL okay = [session startTunnelWithOptions:[NSDictionary<NSString*,id> new]
                                andReturnError:&error];
  if (error) {
    logger.warning() << "proxy start error:" << error.localizedDescription;
  } else if (!okay) {
    logger.warning() << "proxy start failed";
  } else {
    // Save the session and retain it.
    [session retain];
    m_session = session;
  }

  return true;
}

void FirewallUtilsMacos::disable() {
  logger.debug() << "MacOS Firewall disabled";
  if (m_session) {
    NETunnelProviderSession* session =
        static_cast<NETunnelProviderSession*>(m_session);
    // Stop the split tunnel proxy.
    [session stopTunnel];
    [session release];
    m_session = nullptr;
  }
}

bool FirewallUtilsMacos::updatePeer(const InterfaceConfig& config) {
#if 0
  if (m_session) {
    return true;
  }

  auto msg = [[NSKeyedArchiver alloc] initRequiringSecureCoding:true];
  [msg encodeObject:@"addBypass"
             forKey:@"action"];
  [msg finishEncoding];

  NSError* error;
  auto session = static_cast<NETunnelProviderSession*>(m_session);
  bool result = [session sendProviderMessage:[msg encodedData]
                                 returnError:&error
                             responseHandler:nil];
  if (error != nil) {
    logger.warning() << "update peer failed:" << error.localizedDescription;
  }
  return result;
#else
  return true;
#endif
}

void FirewallUtilsMacos::deletePeer(const InterfaceConfig& config) {
#if 0
  if (m_session) {
    return;
  }

  auto msg = [[NSKeyedArchiver alloc] initRequiringSecureCoding:true];
  [msg encodeObject:@"deleteBypass"
             forKey:@"action"];
  [msg finishEncoding];

  NSError* error;
  auto session = static_cast<NETunnelProviderSession*>(m_session);
  bool result = [session sendProviderMessage:[msg encodedData]
                                 returnError:&error
                             responseHandler:nil];
  if (error != nil) {
    logger.warning() << "delete peer error:" << error.localizedDescription;
  } else if (!result) {
    logger.warning() << "delete peer failed";
  }
#endif
}

// Split tunnel is supported if we loaded the extension successfully.
bool FirewallUtilsMacos::splitTunnelSupported() const {
  auto loader = static_cast<MacosSplitTunnelLoader*>(m_loader);
  if (loader.manager == nil) {
    return false;
  }
  return loader.manager.enabled;
}
