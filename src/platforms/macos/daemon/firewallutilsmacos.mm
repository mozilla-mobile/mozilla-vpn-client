/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "firewallutilsmacos.h"

#include "leakdetector.h"
#include "logger.h"

#import <Foundation/Foundation.h>
#import <NetworkExtension/NetworkExtension.h>
#import <SystemExtensions/SystemExtensions.h>

#include <QThread>

namespace {
Logger logger("FirewallUtilsMacos");
}  // namespace

@interface MacosSplitTunnelLoader : NSObject<OSSystemExtensionRequestDelegate>
@property(readonly) NSString* identifier;

@property FirewallUtilsMacos* parent;
@property(retain) NETransparentProxyManager* manager;

- (id) initWithObject:(FirewallUtilsMacos*)parent;

@end

FirewallUtilsMacos::FirewallUtilsMacos(Daemon* daemon) : FirewallUtils(daemon) {
  MZ_COUNT_CTOR(FirewallUtilsMacos);
  logger.debug() << "FirewallUtilsMacos created.";
  m_daemon = daemon;

  // Create the Obj-C loader class.
  MacosSplitTunnelLoader* loader = [MacosSplitTunnelLoader alloc];
  [loader initWithObject:this];
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

// Split tunnel is supported if we loaded the extension successfully.
bool FirewallUtilsMacos::splitTunnelSupported() const {
  auto loader = static_cast<MacosSplitTunnelLoader*>(m_loader);
  if (loader.manager == nil) {
    return false;
  }
  return loader.manager.enabled;
}

@implementation MacosSplitTunnelLoader

- (NSString*) identifier {
  NSString* appId = [[NSBundle mainBundle] bundleIdentifier];
  return [appId stringByAppendingString:@".split-tunnel"];
}

- (id)initWithObject:(FirewallUtilsMacos*)parent {
  self = [super init];
  self.parent = parent;
  self.manager = nullptr;
  return self;
}

- (void) request:(OSSystemExtensionRequest *) request
didFailWithError:(NSError *) error {
  logger.warning() << "activation request failed:" << error.localizedDescription;
}

- (void) requestNeedsUserApproval:(OSSystemExtensionRequest *) request {
  logger.warning() << "activation request needs user approval";
}

- (OSSystemExtensionReplacementAction) request:(OSSystemExtensionRequest *) request
                   actionForReplacingExtension:(OSSystemExtensionProperties *) existing
                                 withExtension:(OSSystemExtensionProperties *) ext {
  logger.warning() << "activation replacement action:" << existing.bundleVersion
                   << "->" << ext.bundleVersion;
  return OSSystemExtensionReplacementActionReplace;
}

- (void)    request:(OSSystemExtensionRequest *) request
didFinishWithResult:(OSSystemExtensionRequestResult) result {
  logger.info() << "activation request complete:" << result;
  [self setupProxy:^(NSError* error){
    if (error != nil) {
      logger.info() << "proxy setup failed:" << error.localizedDescription;
    } else {
      logger.info() << "proxy setup complete";
    }
  }];
}

- (void) setupManager:(void (^)(NSError * error)) completionHandler {
  [NETransparentProxyManager loadAllFromPreferencesWithCompletionHandler:^(NSArray<NETransparentProxyManager *>* managers, NSError* err){
    if (err != nil) {
      logger.warning() << "proxy manager load failed:"
                       << err.localizedDescription;
      completionHandler(err);
      return;
    }

    // Check if an existing manager can be used.
    for (NETransparentProxyManager* mgr in managers) {
      NETunnelProviderProtocol* proto =
          static_cast<NETunnelProviderProtocol*>([mgr protocolConfiguration]);
      if ([proto.providerBundleIdentifier isEqualToString:self.identifier]) {
        logger.info() << "proxy manager found for:"
                      << proto.providerBundleIdentifier;
        self.manager = mgr;
        completionHandler(nil);
        return;
      }
    }

    // Otherwise - create a new manager.
    auto protocol = [NETunnelProviderProtocol new];
    protocol.providerBundleIdentifier = self.identifier;
    protocol.serverAddress = @"127.0.0.1";

    self.manager = [NETransparentProxyManager new];
    self.manager.protocolConfiguration = protocol;
    self.manager.localizedDescription = @"Mozilla VPN Split Tunnel";
    logger.info() << "proxy manager created for:"
                  << protocol.providerBundleIdentifier;
    completionHandler(nil);
  }];
}

- (void) setupProxy:(void (^)(NSError* error)) completionHandler {
  [self setupManager:^(NSError* error){
    if (error != nil) {
      completionHandler(error);
      return;
    }

    // Enable the manager and sync preferences
    self.manager.enabled = true;
    [self.manager saveToPreferencesWithCompletionHandler:^(NSError* saveErr){
      if (saveErr != nil) {
        logger.debug() << "proxy prefs setup failed:"
                       << saveErr.localizedDescription;
      }
      [self.manager loadFromPreferencesWithCompletionHandler:^(NSError* loadErr){
        if (loadErr != nil) {
          logger.debug() << "proxy prefs load failed:"
                         << loadErr.localizedDescription;
        }
        completionHandler(nil);
      }];
    }];
  }];
}

@end
