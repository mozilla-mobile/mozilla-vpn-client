/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macossplittunnelloader.h"

#include "logger.h"

#import <Foundation/Foundation.h>
#import <NetworkExtension/NetworkExtension.h>
#import <SystemExtensions/SystemExtensions.h>

namespace {
Logger logger("MacosSplitTunnelLoader");
}  // namespace

@implementation MacosSplitTunnelLoader

- (NSString*) identifier {
  NSString* appId = [[NSBundle mainBundle] bundleIdentifier];
  return [appId stringByAppendingString:@".split-tunnel"];
}

- (id)init {
  self = [super init];
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
  [self setupManager:^(NSError* error){
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
        break;
      }
    }
    // Otherwise - create a new manager.
    if (self.manager == nil) {
      self.manager = [NETransparentProxyManager new];
      self.manager.localizedDescription = @"Mozilla VPN Split Tunnel";
      logger.info() << "proxy manager created for:" << self.identifier;
    }

    // Update the tunnel configuration.
    auto protocol = [NETunnelProviderProtocol new];
    protocol.providerBundleIdentifier = self.identifier;
    protocol.serverAddress = @"127.0.0.1";
    self.manager.protocolConfiguration = protocol;

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
