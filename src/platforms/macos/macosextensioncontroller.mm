/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosextensioncontroller.h"

#import <Foundation/Foundation.h>
#import <NetworkExtension/NetworkExtension.h>
#import <SystemExtensions/SystemExtensions.h>

#include <QMetaMethod>

#include "logger.h"
#include "macosutils.h"

// An extension loader - used to forward Obj-C messages back to Qt.
@interface MacOSExtensionDelegate : NSObject <OSSystemExtensionRequestDelegate>
@property MacOSExtensionController* parent;
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
}

MacOSExtensionController::~MacOSExtensionController() {
  [m_delegate release];
}

void MacOSExtensionController::initialize(const Device* device, const Keys* keys) {
  // Create a request to install the system extension.
  dispatch_queue_t queue =
      dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
  OSSystemExtensionRequest* req =
      [OSSystemExtensionRequest activationRequestForExtension: extIdentifier()
                                                        queue: queue];
  req.delegate = m_delegate;

  // Start the request
  logger.debug() << "activation request started:" << req.identifier;
  [[OSSystemExtensionManager sharedManager] submitRequest: req];
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

    // Register the delegate to receive updates when the extension state is changed.
    NSNotificationCenter* notify = [NSNotificationCenter defaultCenter];
    [notify addObserver:m_delegate
               selector:@selector(notifyEnabledChanged:)
                   name:NEVPNConfigurationChangeNotification
                 object:m_manager];

    // Enable the manager and sync preferences
    m_manager.enabled = true;
    [m_manager saveToPreferencesWithCompletionHandler:^(NSError* saveErr){
      if (saveErr != nil) {
        logger.debug() << "proxy prefs setup failed:"
                       << saveErr.localizedDescription;
      }
      [m_manager loadFromPreferencesWithCompletionHandler:^(NSError* loadErr){
        if (loadErr != nil) {
          logger.debug() << "proxy prefs load failed:"
                         << loadErr.localizedDescription;
        }
        NEVPNConnection* conn = m_manager.connection;
        [notify addObserver:m_delegate
                   selector:@selector(notifyStatusChanged:)
                       name:NEVPNStatusDidChangeNotification
                     object:conn];

        if (conn.status == NEVPNStatusConnected) {
          emit initialized(true, true, QDateTime::fromCFDate((CFDateRef)conn.connectedDate));
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
    emit disconnected();
    // Log the disconnection error, if any.
    if (!m_session) {
      return;
    }
    [m_session fetchLastDisconnectErrorWithCompletionHandler:^(NSError *err){
      if (err) {
        logger.warning() << "tunnel disconnected:" << err;
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
  [m_manager saveToPreferencesWithCompletionHandler:^(NSError* error) {
    if (error) {
      logger.warning() << "prefs update error:" << error.localizedDescription;
      return;
    }

    // If we don't already have a session - start one.
    if (m_session) {
      return;
    }
    NETunnelProviderSession* session =
        static_cast<NETunnelProviderSession*>(m_manager.connection);
    BOOL okay = [session startTunnelWithOptions:options andReturnError:&error];
    if (error) {
      logger.warning() << "proxy start error:" << error.localizedDescription;
    } else if (!okay) {
      logger.warning() << "proxy start failed";
    } else {
      // Save the session and retain it.
      m_session = [session retain];
    }
  }];
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

- (void) request:(OSSystemExtensionRequest *) request
didFailWithError:(NSError *) error {
  QMetaObject::invokeMethod(self.parent, "extLoaderFailure");
                            Q_ARG(QString, QString::fromNSString(error.localizedDescription));
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

@end
