/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosextensioncontroller.h"

#import <Foundation/Foundation.h>
#import <NetworkExtension/NetworkExtension.h>
#import <ServiceManagement/ServiceManagement.h>
#import <SystemExtensions/SystemExtensions.h>

#include <QMetaMethod>

#include "logger.h"
#include "macosutils.h"

// An extension loader - used to forward Obj-C messages back to Qt.
@interface MacOSExtensionDelegate : NSObject <OSSystemExtensionRequestDelegate, OSSystemExtensionsWorkspaceObserver>
@property MacOSExtensionController* parent;
@property (readonly, getter=getBundleVersion) NSString* bundleVersion;
@property (readonly, getter=getQueue) dispatch_queue_t queue;

- (id)initWithObject:(MacOSExtensionController*)controller;
- (void)notifyEnabledChanged:(NSNotification*)notify;
- (void)notifyStatusChanged:(NSNotification*)notify;
@end

namespace {
Logger logger("MacOSExtensionController");
}  // namespace

MacOSExtensionController::MacOSExtensionController() : ControllerImpl()  {
  // Create the system extension loader delegate.
  m_delegate = [[MacOSExtensionDelegate alloc] initWithObject:this];
  [m_delegate retain];

  // For macOS 15.1 and beyond - observe system extension status changes.
  if (@available(macOS 15.1, *)) {
    NSError* err = nil;
    OSSystemExtensionsWorkspace* workspace = [OSSystemExtensionsWorkspace sharedWorkspace];
    if (![workspace addObserver:m_delegate error:&err]) {
      logger.warning() << "sysex observer error:" << err;
    }
  }
}

MacOSExtensionController::~MacOSExtensionController() {
  [m_delegate release];
}

void MacOSExtensionController::initialize(const Device* device, const Keys* keys) {
  // Fetch the currently installed system extensions.
  OSSystemExtensionRequest* req =
      [OSSystemExtensionRequest propertiesRequestForExtension: extIdentifier()
                                                        queue: m_delegate.queue];

  // Start the request
  logger.debug() << "property request started:" << req.identifier;
  req.delegate = m_delegate;
  [[OSSystemExtensionManager sharedManager] submitRequest: req];

  // Attempt to register the socks proxy tool.
  NSError* err = nil;
  NSString* proxyPlistName = MacOSUtils::appId(".socksproxy.plist").toNSString();
  SMAppService* proxy = [SMAppService agentServiceWithPlistName:proxyPlistName];
  if (![proxy registerAndReturnError:&err]) {
    logger.debug() << "socks proxy agent registration failed:" << err;
  } else {
    logger.debug() << "socks proxy agent registration successful";
  }
}

NSString* MacOSExtensionController::extIdentifier() {
  return MacOSUtils::appId(".network-extension").toNSString();
}

void MacOSExtensionController::extLoaderSuccess(int result) {
  logger.info() << "activation request complete:" << result;

  // Start by loading all the proxy managers.
  [NETransparentProxyManager loadAllFromPreferencesWithCompletionHandler:^(NSArray<NETransparentProxyManager *>* managers, NSError* err){
    if (err != nil) {
      logger.debug() << "activation setup failed:" << err;
      emit initialized(false, false, QDateTime());
      return;
    }

    // Check if an existing manager can be used.
    NSString* extId = MacOSExtensionController::extIdentifier();
    for (NETransparentProxyManager* mgr in managers) {
      if (![mgr.protocolConfiguration isKindOfClass:[NETunnelProviderProtocol class]]) {
        continue;
      }
      NETunnelProviderProtocol* proto =
          static_cast<NETunnelProviderProtocol*>(mgr.protocolConfiguration);
      if ([proto.providerBundleIdentifier isEqualToString:extId]) {
        logger.info() << "proxy manager found for:" << proto.providerBundleIdentifier;
        m_manager = mgr;
        break;
      }
    }

    // Otherwise - create a new manager.
    if (m_manager == nil) {
      m_manager = [NETransparentProxyManager new];
      m_manager.localizedDescription = @"Mozilla VPN Network Extension";
      logger.info() << "proxy manager created for:" << extId;
    }

    // Update the tunnel configuration.
    auto protocol = [NETunnelProviderProtocol new];
    protocol.providerBundleIdentifier = extId;
    protocol.serverAddress = @"127.0.0.1";
    m_manager.protocolConfiguration = protocol;
    m_manager.enabled = true;

    // Register the delegate to receive updates when the extension state is changed.
    NSNotificationCenter* notify = [NSNotificationCenter defaultCenter];
    [notify addObserver:m_delegate
               selector:@selector(notifyEnabledChanged:)
                   name:NEVPNConfigurationChangeNotification
                 object:m_manager];

    [notify addObserver:m_delegate
               selector:@selector(notifyStatusChanged:)
                   name:NEVPNStatusDidChangeNotification
                 object:m_manager.connection];

    // Sync the manager to preferences
    [m_manager saveToPreferencesWithCompletionHandler:^(NSError* saveErr){
      if (saveErr != nil) {
        // We still consider the extension loaded - and thus the permission
        // request state can be passed. However, setup is not yet fully
        // complete and we will have to retry on the next activation.
        logger.debug() << "proxy prefs setup failed:"
                       << saveErr.localizedDescription;
        emit initialized(true, false, QDateTime());
        return;
      }

      // Re-load the connecion manager from prefernces to complete the sync.
      [m_manager loadFromPreferencesWithCompletionHandler:^(NSError* loadErr){
        if (loadErr != nil) {
          logger.debug() << "proxy prefs load failed:"
                         << loadErr.localizedDescription;
        }

        if (m_manager.connection.status == NEVPNStatusConnected) {
          CFDateRef date = (CFDateRef)m_manager.connection.connectedDate;
          emit initialized(true, true, QDateTime::fromCFDate(date));
        } else {
          emit initialized(true, false, QDateTime());
        }
      }];
    }];
  }];
}

void MacOSExtensionController::extLoaderFailure(const QString& reason) {
  logger.warning() << "activation request failed:" << reason;
}

void MacOSExtensionController::extNeedsApproval() {
  logger.warning() << "activation request needs user approval";
  emit permissionRequired();
}

void MacOSExtensionController::extEnabledChange(bool enabled) {
  logger.warning() << "activation enable changed:" << enabled;
}

void MacOSExtensionController::extStatusChange(int status) {
  logger.warning() << "connection status changed:" << status;
  if (status == NEVPNStatusConnected) {
    emit connected(m_serverPublicKey);
  }
  if (status == NEVPNStatusDisconnected) {
    // Log the disconnection error, if any.
    if (!m_session) {
      emit disconnected();
      return;
    }
    [m_session fetchLastDisconnectErrorWithCompletionHandler:^(NSError *err){
      auto guard = qScopeGuard([this](){ emit disconnected(); });
      if (!err) {
        return;
      }
      logger.warning() << "tunnel disconnected:" << err;

      // Special handling for errors originating in the network extension.
      if (![err.domain isEqualToString:extIdentifier()]) {
        return;
      }

      // Special case: If the tunnel closed due to a superseded configuration
      // change, then restart it with the updated configuration.
      if (err.code == NEProviderStopReasonSuperceded) {
        logger.info() << "connection restarting";
        NETunnelProviderProtocol* proto = 
            static_cast<NETunnelProviderProtocol*>(m_manager.protocolConfiguration);
        NSError* startError = nil;
        [m_session startTunnelWithOptions:proto.providerConfiguration
                           andReturnError:&startError];
        if (startError) {
          logger.info() << "connection restart failed:" << startError;
        } else {
          logger.info() << "connection restart succeeded";
          guard.dismiss();
        }
      }
    }];
  }
}

void MacOSExtensionController::activate(const InterfaceConfig& config,
                                        Controller::Reason reason) {
  Q_UNUSED(reason);

  // Create a new tunnel provider session.
  if ((m_manager == nil) || !m_manager.enabled) {
    // Split tunnelling is not supported.
    return;
  }

  // Save the public key for signal emissions.
  m_serverPublicKey = config.m_serverPublicKey;

  // Serialize the interface configuration.
  NSMutableDictionary* options = [NSMutableDictionary dictionary];
  [options setObject:config.m_privateKey.toNSString() forKey:@"privateKey"];
  [options setObject:config.m_deviceIpv4Address.toNSString() forKey:@"deviceIpv4Addr"];
  [options setObject:config.m_deviceIpv6Address.toNSString() forKey:@"deviceIpv6Addr"];
  [options setObject:config.m_serverPublicKey.toNSString() forKey:@"serverPublicKey"];
  [options setObject:config.m_serverIpv4AddrIn.toNSString() forKey:@"serverIpv4AddrIn"];
  [options setObject:config.m_serverIpv6AddrIn.toNSString() forKey:@"serverIpv6AddrIn"];
  [options setObject:config.m_serverIpv4Gateway.toNSString() forKey:@"serverIpv4Gateway"];
  [options setObject:config.m_serverIpv6Gateway.toNSString() forKey:@"serverIpv6Gateway"];
  [options setObject:[NSNumber numberWithInt:config.m_serverPort] forKey:@"serverPort"];
  if (!config.m_dnsServer.isEmpty()) {
    [options setObject:@[config.m_dnsServer.toNSString()] forKey:@"dnsSettings"];
  }

  NSMutableArray* ipAddressRanges =
      [NSMutableArray arrayWithCapacity:config.m_allowedIPAddressRanges.length()];
  for (const IPAddress& range : config.m_allowedIPAddressRanges) {
    [ipAddressRanges addObject:range.toString().toNSString()];
  }
  [options setObject:ipAddressRanges forKey:@"routes"];

  // Serialize the excluded application list.
  NSMutableArray* vpnDisabledApps =
      [NSMutableArray arrayWithCapacity:config.m_vpnDisabledApps.length()];
  for (const QString& appId : config.m_vpnDisabledApps) {
    [vpnDisabledApps addObject:appId.toNSString()];
  }
  [options setObject:vpnDisabledApps forKey:@"apps"];

  // Update the tunnel configuration.
  NETunnelProviderProtocol* proto =
      static_cast<NETunnelProviderProtocol*>(m_manager.protocolConfiguration);
  proto.providerConfiguration = options;
  [m_manager saveToPreferencesWithCompletionHandler:^(NSError* saveErr) {
    if (saveErr) {
      logger.warning() << "prefs update error:" << saveErr;
      return;
    }

    // Start the tunnel if the extension configuration exists.
    if (m_manager.connection.status != NEVPNStatusInvalid) {
      startTunnel();
      return;
    }

    // We must re-load from preferences first.
    logger.debug() << "extension sync required:" << m_manager.connection.status;
    [m_manager loadFromPreferencesWithCompletionHandler:^(NSError* loadErr) {
      if (loadErr) {
        logger.warning() << "prefs sync error:" << loadErr;
      } else {
        startTunnel();
      }
    }];
  }];
}

void MacOSExtensionController::startTunnel() {
  if (m_session != nil) {
    return;
  }

  NETunnelProviderProtocol* proto =
      static_cast<NETunnelProviderProtocol*>(m_manager.protocolConfiguration);
  NETunnelProviderSession* session =
      static_cast<NETunnelProviderSession*>(m_manager.connection);

  NSError* error = nil;
  BOOL okay = [session startTunnelWithOptions:proto.providerConfiguration
                               andReturnError:&error];
  if (error) {
    logger.warning() << "proxy start error:" << error.localizedDescription;
  } else if (!okay) {
    logger.warning() << "proxy start failed";
  } else {
    // Save the session and retain it.
    m_session = [session retain];
  }
}

void MacOSExtensionController::deactivate() {
  if (m_session) {
    // Stop the split tunnel proxy.
    [m_session stopTunnel];
    [m_session release];
    m_session = nullptr;
  }
}

QString MacOSExtensionController::parseArchivedString(NSCoder* archive,
                                                      NSString* key) {
  NSString* s = [archive decodeObjectOfClass:[NSString class] forKey:key];
  return QString::fromNSString(s);
}

QHostAddress MacOSExtensionController::parseArchivedAddress(NSCoder* archive,
                                                            NSString* key) {
  return QHostAddress(parseArchivedString(archive, key));
}

void MacOSExtensionController::checkStatus() {
  if (!m_session) {
    // Extension is not running.
    return;
  }

  NSKeyedArchiver* msg = [[NSKeyedArchiver alloc] initRequiringSecureCoding:YES];
  [msg encodeObject:@"status" forKey:@"action"];
  [msg finishEncoding];

  NSError* error = nil;
  [m_session sendProviderMessage:msg.encodedData
                     returnError:&error
                 responseHandler:^(NSData* response){
    NSError* decodeError = nil;
    NSKeyedUnarchiver* archive = [NSKeyedUnarchiver alloc];
    if (![archive initForReadingFromData:response error:&decodeError]) {
      logger.debug() << "status decode failed:" << decodeError;
      return;
    }

    ControllerStatus st;
    NSDate* timestamp = [archive decodeObjectOfClass:[NSDate class] forKey:@"lastHandshake"];
    if (timestamp) {
      st.m_connected = true; 
      st.m_timestamp = QDateTime::fromCFDate((CFDateRef)timestamp);
    }
    st.m_ipv4Gateway = parseArchivedAddress(archive, @"ipv4gateway");
    st.m_ipv6Gateway = parseArchivedAddress(archive, @"ipv6gateway");
    st.m_ipv4Address = parseArchivedAddress(archive, @"ipv4address");
    st.m_ipv6Address = parseArchivedAddress(archive, @"ipv6address");
    st.m_rxBytes = [archive decodeInt64ForKey:@"rxBytes"];
    st.m_txBytes = [archive decodeInt64ForKey:@"txBytes"];
    emit statusUpdated(st);
  }];

  if (error != nil) {
    logger.debug() << "status request failed:" << error;
    emit statusUpdated(ControllerStatus());
    return;
  }
}

@implementation MacOSExtensionDelegate
- (id)initWithObject:(MacOSExtensionController*)controller {
  self = [super init];
  self.parent = controller;
  return self;
}

- (dispatch_queue_t) getQueue {
  return dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
}

- (NSString*) getBundleVersion {
  NSBundle* appBundle = [NSBundle mainBundle];
  return [appBundle objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
}

- (void) request:(OSSystemExtensionRequest *) request
 foundProperties:(NSArray<OSSystemExtensionProperties *> *) properties {
#ifdef MZ_DEBUG
  // Log the discovered extensions for debugging purposes.
  logger.debug() << "sysex properties for:" << request.identifier;
  for (OSSystemExtensionProperties* ext in properties) {
    if ([ext.bundleIdentifier compare:request.identifier] != NSOrderedSame) {
      continue;
    }

    logger.debug() << "sysex:" << ext.URL.path;
    auto log = logger.debug();
    log << "version:" << ext.bundleVersion;
    if (ext.isUninstalling) {
      log << "uninstall";
    }
    if (ext.isAwaitingUserApproval) {
      log << "await";
    }
    if (ext.isEnabled) {
      log << "enabled";
    }
  }
#endif

  for (OSSystemExtensionProperties* ext in properties) {
    // Ignore extensions with a different identifier or version.
    if ([ext.bundleIdentifier compare:request.identifier] != NSOrderedSame) {
      continue;
    }
    if ([ext.bundleVersion compare:self.bundleVersion] != NSOrderedSame) {
      continue;
    }
    if (ext.isUninstalling) {
      continue;
    }

    if (ext.isEnabled) {
      QMetaObject::invokeMethod(self.parent, "extLoaderSuccess", Q_ARG(int, 0));
      return;
    } else if (@available(macOS 15.1, *)) {
      QMetaObject::invokeMethod(self.parent, "extNeedsApproval");
      return;
    } else if (ext.isAwaitingUserApproval) {
      QMetaObject::invokeMethod(self.parent, "extNeedsApproval");
      return;
    }
  }

  // Otherwise, we were unable to find a matching extension. Start a request
  // to install (or reinstall) the VPN network extension.
  OSSystemExtensionRequest* activationRequest =
      [OSSystemExtensionRequest activationRequestForExtension: request.identifier
                                                        queue: self.queue];

  logger.debug() << "activation request started:" << request.identifier;
  activationRequest.delegate = self;
  [[OSSystemExtensionManager sharedManager] submitRequest: activationRequest];
}

- (void) request:(OSSystemExtensionRequest *) request
didFailWithError:(NSError *) error {
  QMetaObject::invokeMethod(self.parent, "extLoaderFailure",
                            Q_ARG(QString, QString::fromNSString(error.localizedDescription)));
}

- (void) requestNeedsUserApproval:(OSSystemExtensionRequest *) request {
  QMetaObject::invokeMethod(self.parent, "extNeedsApproval");
}

- (OSSystemExtensionReplacementAction) request:(OSSystemExtensionRequest *) request
                   actionForReplacingExtension:(OSSystemExtensionProperties *) existing
                                 withExtension:(OSSystemExtensionProperties *) ext {
  logger.warning() << "extension replacement action:" << existing.bundleVersion
                   << "->" << ext.bundleVersion;
  return OSSystemExtensionReplacementActionReplace;
}

- (void)    request:(OSSystemExtensionRequest *) request
didFinishWithResult:(OSSystemExtensionRequestResult) result {
  QMetaObject::invokeMethod(self.parent, "extLoaderSuccess", Q_ARG(int, result));
}

- (void)notifyEnabledChanged:(NSNotification*)notify {
  bool enabled = static_cast<NETransparentProxyManager*>(notify.object).enabled;
  QMetaObject::invokeMethod(self.parent, "extEnabledChange", Q_ARG(bool, enabled));
}

- (void)notifyStatusChanged:(NSNotification*)notify {
  NEVPNConnection* conn = static_cast<NEVPNConnection*>(notify.object);
  QMetaObject::invokeMethod(self.parent, "extStatusChange", Q_ARG(int, conn.status));
}

// These observation methods are only supported for macOS 15.1 and beyond.
- (void)systemExtensionWillBecomeDisabled:(OSSystemExtensionInfo *)info API_AVAILABLE(macos(15.1)) {
  logger.debug() << "sysex disabled:" << info.bundleIdentifier << "version:" << info.bundleVersion;
}

- (void)systemExtensionWillBecomeEnabled:(OSSystemExtensionInfo *)info API_AVAILABLE(macos(15.1)) {
  logger.debug() << "sysex enabled:" << info.bundleIdentifier << "version:" << info.bundleVersion;
  QMetaObject::invokeMethod(self.parent, "extLoaderSuccess", Q_ARG(int, 0));
}

- (void)systemExtensionWillBecomeInactive:(OSSystemExtensionInfo *)info API_AVAILABLE(macos(15.1)) {
  logger.debug() << "sysex inactive:" << info.bundleIdentifier << "version:" << info.bundleVersion;
}

@end
