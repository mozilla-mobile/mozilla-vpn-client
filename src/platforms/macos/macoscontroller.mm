/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macoscontroller.h"

#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScopeGuard>
#include <QVersionNumber>

#include "constants.h"
#include "feature/feature.h"
#include "glean/generated/metrics.h"
#include "logger.h"
#include "macossplittunnelloader.h"
#include "macosutils.h"
#include "version.h"
#include "xpcdaemonprotocol.h"

#import <Cocoa/Cocoa.h>
#import <NetworkExtension/NetworkExtension.h>
#import <Security/Authorization.h>
#import <Security/AuthorizationTags.h>
#import <ServiceManagement/ServiceManagement.h>
#import <SystemExtensions/SystemExtensions.h>

namespace {
Logger logger("MacOSController");
}  // namespace

constexpr const int SERVICE_INIT_POLL_INTERVAL_MSEC = 1000;

// A delegate object used to receive async events from the daemon.
@interface XpcClientDelegate : NSObject<XpcClientProtocol>
@property ControllerImpl* parent;
- (id)initWithObject:(ControllerImpl*)controller;
@end

MacOSController::MacOSController() : ControllerImpl()  {
  m_registerTimer.setSingleShot(true);
  connect(&m_registerTimer, &QTimer::timeout, this,
          &MacOSController::registerService);
  
  m_connectTimer.setSingleShot(true);
  connect(&m_connectTimer, &QTimer::timeout, this,
          &MacOSController::connectService);

  // Load the system extension if the networkExtension feature is enabled.
  if (Feature::get(Feature::Feature_networkExtension)->isSupported()) {
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
}

MacOSController::~MacOSController() {
  if (m_connection != nil) {
    NSXPCConnection* conn = static_cast<NSXPCConnection*>(m_connection);
    [conn invalidate];
    [conn release];
  }

  if (m_loader) {
    [static_cast<MacosSplitTunnelLoader*>(m_loader) release];
  }
}

NSString* MacOSController::plist() const {
  return MacOSUtils::appId(".xpc-daemon.plist").toNSString();
}

NSString* MacOSController::machServiceName() const {
  return MacOSUtils::appId(".xpc-daemon").toNSString();
}

void MacOSController::initialize(const Device* device, const Keys* keys) {
  // Setup a temporary connection to check the daemon version.
  NSXPCConnection* conn = [NSXPCConnection alloc];
  [conn initWithMachServiceName:machServiceName()
                        options:NSXPCConnectionPrivileged];
  conn.remoteObjectInterface =
      [NSXPCInterface interfaceWithProtocol:@protocol(XpcDaemonProtocol)];
  [conn activate];

  // Create the remote object.
  NSObject<XpcDaemonProtocol>* remote = nullptr;
  remote = [conn remoteObjectProxyWithErrorHandler:^(NSError* error){
    logger.debug() << "Initialize daemon failed:" << error.localizedDescription;
    QMetaObject::invokeMethod(this, &MacOSController::upgradeService);
  }];

  // Get the daemon version and decide if an upgrade is needed.
  [remote getVersion:^(NSString* version){
    logger.debug() << "Initialize daemon version:" << version;
    QVersionNumber clientVersion =
        QVersionNumber::fromString(QCoreApplication::applicationVersion());
    QVersionNumber daemonVersion =
        QVersionNumber::fromString(QString::fromNSString(version));
    if (daemonVersion < clientVersion) {
      QMetaObject::invokeMethod(this, &MacOSController::upgradeService);
    } else {
      QMetaObject::invokeMethod(this, &MacOSController::connectService);
    }
  }];
}

void MacOSController::upgradeService() {
  // Attempt to register the daemon using the SMAppService interface.
  SMAppService* service = [SMAppService daemonServiceWithPlistName:plist()];

  // If the service purports to be installed, but the local socket doesn't
  // exist. We might need to forcibly remove the old daemon and upgrade it.
  // This can occur when upgrading from a legacy launchd-style service to
  // one that is managed by SMAppService.
  [service unregisterWithCompletionHandler:^(NSError* error){
    if (error != nil) {
      logger.warning() << "Legacy service removal failed:" << error;
    } else {
      logger.info() << "Legacy service removal succeeded";
    }
    QMetaObject::invokeMethod(this, &MacOSController::registerService);
  }];
}

void MacOSController::registerService(void) {
  SMAppService* service = [SMAppService daemonServiceWithPlistName:plist()];

  // Attempt to register the service upon initialization. This should be a
  // no-op if the service is already registered.
  NSError* error = nil;
  if ([service registerAndReturnError: &error]) {
    logger.debug() << "Mozilla VPN daemon registered successfully.";
  } else if (error.code == kSMErrorInvalidSignature) {
    // If the build is unsigned, continue anyways and hope for the best.
    // This is to mitigate developer pain by allowing the VPN to make use
    // of a pre-existing daemon from a signed installation.
    logger.error() << "Unable to register Mozilla VPN daemon:"
                    << "code signature invalid";
    connectService();
    return;
  } else {
    // Otherwise, we encountered some other error. Most likely the user
    // needs to approve the daemon to run. Which will be handled below.
    logger.error() << "Unable to register Mozilla VPN daemon:"
                    << error.localizedDescription;
  }

  // Check the service status for how to proceed.
  switch ([service status]) {
    case SMAppServiceStatusNotRegistered:
      logger.debug() << "Mozilla VPN daemon not registered.";
      m_registerTimer.start(SERVICE_INIT_POLL_INTERVAL_MSEC);
      break;

    case SMAppServiceStatusNotFound:
      logger.debug() << "Mozilla VPN daemon not found.";
      break;

    case SMAppServiceStatusEnabled:
      logger.debug() << "Mozilla VPN daemon enabled.";
      m_permissionRequired = false;
      connectService();
      break;

    case SMAppServiceStatusRequiresApproval:
      logger.debug() << "Mozilla VPN daemon requires approval.";
      if (!m_permissionRequired) {
        m_permissionRequired = true;
        emit permissionRequired();
      }
      m_registerTimer.start(SERVICE_INIT_POLL_INTERVAL_MSEC);
      break;
  }
}

void MacOSController::connectService(void) {
  // Create an XPC connection to the daemon.
  NSXPCConnection* conn = [NSXPCConnection alloc];
  [conn initWithMachServiceName:machServiceName()
                        options:NSXPCConnectionPrivileged];
  conn.remoteObjectInterface =
      [NSXPCInterface interfaceWithProtocol:@protocol(XpcDaemonProtocol)];
  [conn activate];

  // Make an XPC request to get the status.
  // We may need to re-activate the connection here.
  NSObject<XpcDaemonProtocol>* remote = nullptr;
  remote = [conn remoteObjectProxyWithErrorHandler:^(NSError* error) {
    logger.debug() << "daemon connection failed:" << error.localizedDescription;

    // On a third failure, record Glean telemetry that can be used for alerting.
    if (m_connectAttempt < 3) {
      m_connectAttempt = m_connectAttempt + 1;
      if (m_connectAttempt == 3) {
        mozilla::glean::performance::DaemonConnectionErrorExtra extras;
        extras._platform = "macOS";
        mozilla::glean::performance::daemon_connection_error.record(extras);
      };
    }
    QMetaObject::invokeMethod(&m_connectTimer, "start",
                              Q_ARG(int, SERVICE_INIT_POLL_INTERVAL_MSEC));
  }];

  // Get the status and report the controller as initialized.
  [remote getStatus:^(NSString* status){
    // Save the connection in the controller class for further use.
    [conn retain];
    m_connection = conn;
    m_connectTimer.stop();

    // Export the client object to receive async events.
    XpcClientDelegate* delegate = [XpcClientDelegate alloc];
    conn.exportedObject = [delegate initWithObject:this];
    conn.exportedInterface =
        [NSXPCInterface interfaceWithProtocol:@protocol(XpcClientProtocol)];
    conn.interruptionHandler = ^{
      logger.debug() << "daemon connection interrupted";
      emit backendFailure(Controller::ErrorNone);
      emit disconnected();
    };

    // Inform the rest of the application that initialization is complete.
    QByteArray jsBlob = QString::fromNSString(status).toUtf8();
    QJsonObject jsObj = QJsonDocument::fromJson(jsBlob).object();
    emit initialized(true, jsObj.value("connected").toBool(),
                     QDateTime::fromString(jsObj.value("date").toString()));
    
    // The delegate is now owned by the NSXPCConnection
    [delegate release];
  }];
}

NSObject<XpcDaemonProtocol>* MacOSController::remoteObject() {
  NSXPCConnection* conn = static_cast<NSXPCConnection*>(m_connection);
  return [conn remoteObjectProxyWithErrorHandler:^(NSError* error){
    emit backendFailure(Controller::ErrorNone);
    emit disconnected();
  }];
}

void MacOSController::activate(const InterfaceConfig& config,
                               Controller::Reason reason) {
  QString json = QString::fromUtf8(QJsonDocument(config.toJson()).toJson());
  [remoteObject() activate:json.toNSString()];

  // Create a new tunnel provider session.
  auto loader = static_cast<MacosSplitTunnelLoader*>(m_loader);
  if (!loader || (loader.manager == nil) || !loader.manager.enabled) {
    // Split tunnelling is not supported.
    return;
  }

  // Serialize the interface configuration.
  NSMutableDictionary* options = [NSMutableDictionary dictionary];
  [options setObject:config.m_privateKey.toNSString() forKey:@"privateKey"];
  [options setObject:config.m_deviceIpv4Address.toNSString() forKey:@"deviceIpv4Address"];
  [options setObject:config.m_deviceIpv6Address.toNSString() forKey:@"deviceIpv6Address"];
  [options setObject:config.m_serverPublicKey.toNSString() forKey:@"serverPublicKey"];
  [options setObject:config.m_serverIpv4AddrIn.toNSString() forKey:@"serverIpv4AddrIn"];
  [options setObject:config.m_serverIpv6AddrIn.toNSString() forKey:@"serverIpv6AddrIn"];
  [options setObject:config.m_serverIpv4Gateway.toNSString() forKey:@"serverIpv4Gateway"];
  [options setObject:config.m_serverIpv6Gateway.toNSString() forKey:@"serverIpv6Gateway"];
  [options setObject:[NSNumber numberWithInt:config.m_serverPort] forKey:@"serverPort"];

  // Get a session and start it.
  NSError* error = nil;
  NETunnelProviderSession* session =
      static_cast<NETunnelProviderSession*>(loader.manager.connection);

  // Start the split tunnel proxy.
  BOOL okay = [session startTunnelWithOptions:options andReturnError:&error];
  if (error) {
    logger.warning() << "proxy start error:" << error.localizedDescription;
  } else if (!okay) {
    logger.warning() << "proxy start failed";
  } else {
    // Save the session and retain it.
    [session retain];
    m_session = session;
  }
}

void MacOSController::deactivate() {
  [remoteObject() deactivate];

  if (m_session) {
    NETunnelProviderSession* session =
        static_cast<NETunnelProviderSession*>(m_session);
    // Stop the split tunnel proxy.
    [session stopTunnel];
    [session release];
    m_session = nullptr;
  }
}

void MacOSController::checkStatus() {
  [remoteObject() getStatus:^(NSString* status){
    QByteArray jsBlob = QString::fromNSString(status).toUtf8();
    QJsonObject obj = QJsonDocument::fromJson(jsBlob).object();
    emitStatusFromJson(obj);
  }];
}

void MacOSController::getBackendLogs(QIODevice* device) {
  NSXPCConnection* conn = [NSXPCConnection alloc];
  [conn initWithMachServiceName:machServiceName()
                        options:NSXPCConnectionPrivileged];
  conn.remoteObjectInterface =
      [NSXPCInterface interfaceWithProtocol:@protocol(XpcDaemonProtocol)];
  [conn activate];

  NSObject<XpcDaemonProtocol>* remote =
      [conn remoteObjectProxyWithErrorHandler:^(NSError*error){
    // Otherwise, try our best to scrape the logs directly off disk.
    QByteArray logData("Failed to open backend logs");
    QFile logfile("/var/log/mozillavpn/mozillavpn.log");
    if (logfile.open(QIODeviceBase::ReadOnly | QIODeviceBase::Text)) {
      logData = logfile.readAll();
    }
    device->write(logData);
    device->close();
  }];

  [remote getBackendLogs:^(NSString* logs){
    NSUInteger length = [logs lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
    device->write(logs.UTF8String, length);
    device->close();
  }];
}

void MacOSController::cleanupBackendLogs() {
  [remoteObject() cleanupBackendLogs];
}

bool MacOSController::splitTunnelSupported() const {
  auto loader = static_cast<MacosSplitTunnelLoader*>(m_loader);
  return (loader.manager != nil) && loader.manager.enabled;
}

bool MacOSController::sendSplitTunnelMessage(const QString& action,
                                             const QStringList& apps) const {
  if (!m_session) {
    logger.debug() << "Split tunneling" << action << "failed: not running";
    return false;
  }

  NSKeyedArchiver* encoder =
      [[NSKeyedArchiver alloc] initRequiringSecureCoding:YES];
  [encoder encodeObject:action.toNSString()
                 forKey:@"action"];

  if (!apps.isEmpty()) {
    NSMutableArray* array = [[NSMutableArray new] init];
    for (const QString& appId : apps) {
      [array addObject:appId.toNSString()];
    }
    [encoder encodeObject:array
                  forKey:@"apps"];
  }

  [encoder finishEncoding];

  NSError* error = nil;
  NETunnelProviderSession* session =
      static_cast<NETunnelProviderSession*>(m_session);
  [session sendProviderMessage:encoder.encodedData
                   returnError:&error
               responseHandler:nil];

  if (error != nil) {
    logger.debug() << "Split tunneling" << action << "failed:" << error;
    return false;
  }
  return true;
}

void MacOSController::forceDaemonCrash() {
  if (m_connection == nullptr) {
    logger.error() << "Daemon does not seem to be running";
    return;
  }
  pid_t pid = [static_cast<NSXPCConnection*>(m_connection) processIdentifier];
  if (pid == 0) {
    logger.error() << "Daemon does not seem to be running";
    return;
  }

  // Create an authorization session.
  AuthorizationRef authRef;
  AuthorizationFlags authFlags =
      kAuthorizationFlagDefaults | kAuthorizationFlagInteractionAllowed |
      kAuthorizationFlagPreAuthorize | kAuthorizationFlagExtendRights;
  OSStatus status = AuthorizationCreate(nullptr, kAuthorizationEmptyEnvironment,
                                        authFlags, &authRef);
  if (status != errAuthorizationSuccess) {
    logger.error() << "Failed to acquire authorization:" << status;
    return;
  }
  auto authGuard = qScopeGuard(
      [&] { AuthorizationFree(authRef, kAuthorizationFlagDefaults); });

  // Acquire execution permissions.
  AuthorizationItem authItems = {kAuthorizationRightExecute, 0, nullptr, 0};
  AuthorizationRights authRights = {1, &authItems};
  status = AuthorizationCopyRights(authRef, &authRights, nullptr, authFlags,
                                   nullptr);
  if (status != errAuthorizationSuccess) {
    logger.error() << "Failed to copy authorization rights:" << status;
    return;
  }

  // Execute 'kill' to terminate the daemon as though it crashed.
  logger.warning() << "Sending SIGSEGV to:" << pid;
  QByteArray pidString = QString::number(pid).toUtf8();
  char killpath[] = "/bin/kill";
  char killsignal[] = "-SEGV";
  char* const killargs[] = {killsignal, pidString.data(), nullptr};

#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
  status = AuthorizationExecuteWithPrivileges(
      authRef, killpath, kAuthorizationFlagDefaults, killargs, nullptr);
#  pragma clang diagnostic pop
  if (status != errAuthorizationSuccess) {
    logger.error() << "Failed to copy execute tool:" << status;
    return;
  }
}

// A delegate object used to receive async events from the daemon.
@implementation XpcClientDelegate
- (id)initWithObject:(ControllerImpl*)controller {
  self = [super init];
  self.parent = controller;
  return self;
}

- (void)connected:(NSString*)pubkey {
  QMetaObject::invokeMethod(self.parent, "connected",
                            Q_ARG(QString, QString::fromNSString(pubkey)));
}

- (void)disconnected {
  QMetaObject::invokeMethod(self.parent, "disconnected");
}

- (void)backendFailure:(NSUInteger)reason {
  Controller::ErrorCode errorCode = static_cast<Controller::ErrorCode>(reason);
  QMetaObject::invokeMethod(self.parent, "backendFailure",
                            Q_ARG(Controller::ErrorCode, errorCode));
}

@end
