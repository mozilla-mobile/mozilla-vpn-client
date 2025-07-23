/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macoscontroller.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScopeGuard>
#include <QVersionNumber>

#include "constants.h"
#include "logger.h"
#include "macosutils.h"
#include "xpcdaemonprotocol.h"
#include "buildinfo.h"

#import <Cocoa/Cocoa.h>
#import <Security/Authorization.h>
#import <Security/AuthorizationTags.h>
#import <ServiceManagement/ServiceManagement.h>

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
}

MacOSController::~MacOSController() {
  if (m_connection != nil) {
    [static_cast<NSXPCConnection*>(m_connection) release];
  }
}

NSString* MacOSController::plist() const {
  return MacOSUtils::appId(".service.plist").toNSString();
}

NSString* MacOSController::machServiceName() const {
  return MacOSUtils::appId(".service").toNSString();
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
    QVersionNumber daemonVersion =
        QVersionNumber::fromString(QString::fromNSString(version));
    if (daemonVersion < QVersionNumber::fromString(BuildInfo::version)) {
      QMetaObject::invokeMethod(this, &MacOSController::upgradeService);
    } else {
      QMetaObject::invokeMethod(this, &MacOSController::connectService);
    }
  }];
}

void MacOSController::upgradeService() {
  // For MacOS 13 and beyond, attempt to register the daemon using the
  // SMAppService interface.
  if (@available(macOS 13, *)) {
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
  } else {
    // Otherwise, for legacy Mac users, they will need to install the daemon
    // some other way. This is normally handled by the installer package.
    return connectService();
  }
}

void MacOSController::registerService(void) {
  if (@available(macOS 13, *)) {
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
  } else {
    // This method should only ever be called for macOS 13 and newer.
    Q_UNREACHABLE();
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
  remote = [conn remoteObjectProxyWithErrorHandler:^(NSError* error){
    logger.debug() << "daemon connection failed:" << error.localizedDescription;
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
}

void MacOSController::deactivate(Controller::Reason reason) {
  [remoteObject() deactivate];
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
