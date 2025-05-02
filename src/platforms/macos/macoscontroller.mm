/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macoscontroller.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include "constants.h"
#include "logger.h"
#include "macosutils.h"
#include "xpcdaemonprotocol.h"

#import <Cocoa/Cocoa.h>
#import <ServiceManagement/ServiceManagement.h>

namespace {
Logger logger("MacOSController");
}  // namespace

constexpr const int SERVICE_REG_POLL_INTERVAL_MSEC = 1000;

// A delegate object used to receive async events from the daemon.
@interface XpcClientDelegate : NSObject<XpcClientProtocol>
@property ControllerImpl* parent;
- (id)initWithObject:(ControllerImpl*)controller;
@end

MacOSController::MacOSController() :
   LocalSocketController(Constants::MACOS_DAEMON_PATH) {

  m_regTimer.setSingleShot(true);
  connect(&m_regTimer, &QTimer::timeout, this,
          &MacOSController::registerService);

  // Create an XPC connection
  QString daemonId = MacOSUtils::appId() + ".daemon";
  XpcClientDelegate* delegate = [XpcClientDelegate alloc];
  NSXPCConnection* conn = [NSXPCConnection alloc];
  [conn initWithMachServiceName:daemonId.toNSString()
                        options:NSXPCConnectionPrivileged];
  conn.exportedObject = [delegate initWithObject:this];
  conn.exportedInterface =
      [NSXPCInterface interfaceWithProtocol:@protocol(XpcClientProtocol)];
  conn.remoteObjectInterface =
      [NSXPCInterface interfaceWithProtocol:@protocol(XpcDaemonProtocol)];

  // Activate the connection and retain it as a member of this object.
  [conn activate];
  [conn retain];
  m_connection = conn;
}

MacOSController::~MacOSController() {
  [static_cast<NSXPCConnection*>(m_connection) release];
}

NSString* MacOSController::plist() const {
  NSString* appId = MacOSUtils::appId();
  Q_ASSERT(appId);
  return [NSString stringWithFormat:@"%@.service.plist", appId];
}

void MacOSController::initialize(const Device* device, const Keys* keys) {
  // For MacOS 13 and beyond, attempt to register the daemon using the
  // SMAppService interface.
  if (@available(macOS 13, *)) {
    SMAppService* service = [SMAppService daemonServiceWithPlistName:plist()];

    // If the service purports to be installed, but the local socket doesn't
    // exist. We might need to forcibly remove the old daemon and upgrade it.
    // This can occur when upgrading from a legacy launchd-style service to
    // one that is managed by SMAppService.
    if ((service.status == SMAppServiceStatusEnabled) &&
         !QFile::exists(Constants::MACOS_DAEMON_PATH)) {
      [service unregisterWithCompletionHandler:^(NSError* error){
        if (error != nil) {
          logger.warning() << "Legacy service removal failed:" << error;
        } else {
          logger.info() << "Legacy service removal succeeded";
        }
        QMetaObject::invokeMethod(this, &MacOSController::registerService);
      }];
      return;
    }

    // Perform the service installation.
    return registerService();
  } else {
    // Otherwise, for legacy Mac users, they will need to install the daemon
    // some other way. This is normally handled by the installer package.
    return LocalSocketController::initialize(device, keys);
  }
}

void MacOSController::registerService(void) {
  if (@available(macOS 13, *)) {
    SMAppService* service = [SMAppService daemonServiceWithPlistName:plist()];

    // Attempt to register the service upon initialization. This should be a
    // no-op if the service is already registered.
    NSError* error = nil;
    if ([service registerAndReturnError: & error]) {
      logger.debug() << "Mozilla VPN daemon registered successfully.";
    } else if (error.code == kSMErrorInvalidSignature) {
      // If the build is unsigned, continue anyways and hope for the best.
      // This is to mitigate developer pain by allowing the VPN to make use
      // of a pre-existing daemon from a signed installation.
      logger.error() << "Unable to register Mozilla VPN daemon:"
                     << "code signature invalid";
      return LocalSocketController::initialize(nullptr, nullptr);
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
        m_regTimer.start(SERVICE_REG_POLL_INTERVAL_MSEC);
        break;

      case SMAppServiceStatusNotFound:
        logger.debug() << "Mozilla VPN daemon not found.";
        break;

      case SMAppServiceStatusEnabled:
        logger.debug() << "Mozilla VPN daemon enabled.";
        m_permissionRequired = false;
        return LocalSocketController::initialize(nullptr, nullptr);

      case SMAppServiceStatusRequiresApproval:
        logger.debug() << "Mozilla VPN daemon requires approval.";
        if (!m_permissionRequired) {
          m_permissionRequired = true;
          emit permissionRequired();
        } 
        m_regTimer.start(SERVICE_REG_POLL_INTERVAL_MSEC);
        break;
    }
  } else {
    // This method should only ever be called for macOS 13 and newer.
    Q_UNREACHABLE();
  }
}

void MacOSController::activate(const InterfaceConfig& config,
                               Controller::Reason reason) {
  QString json = QString::fromUtf8(QJsonDocument(config.toJson()).toJson());
  auto conn = static_cast<NSXPCConnection*>(m_connection);
  NSObject<XpcDaemonProtocol>* obj = [conn remoteObjectProxy];
  [obj activate:json.toNSString()];
}

void MacOSController::deactivate(Controller::Reason reason) {
  auto conn = static_cast<NSXPCConnection*>(m_connection);
  [[conn remoteObjectProxy] deactivate];
}

void MacOSController::checkStatus() {
  [remoteObject() getStatus:^(NSString* status){
    QByteArray jsBlob = QString::fromNSString(status).toUtf8();
    QJsonObject obj = QJsonDocument::fromJson(jsBlob).object();
    emitStatusFromJson(obj);
  }];
}

void MacOSController::getBackendLogs(QIODevice* device) {
  // If the daemon is connected - use the LocalSocketController to fetch logs.
  if (m_daemonState == eReady) {
    LocalSocketController::getBackendLogs(device);
    return;
  }

  // Otherwise, try our best to scrape the logs directly off disk.
  QByteArray logData("Failed to open backend logs");
  QFile logfile("/var/log/mozillavpn/mozillavpn.log");
  if (logfile.open(QIODeviceBase::ReadOnly | QIODeviceBase::Text)) {
    logData = logfile.readAll();
  }
  device->write(logData);
  device->close();
}

// A delegate object used to receive async events from the daemon.
@implementation XpcClientDelegate
- (id)initWithObject:(ControllerImpl*)controller {
  self = [super init];
  self.parent = controller;
  return self;
}

- (void)connected:(NSString*)pubkey {
  logger.debug() << "connected:" << pubkey;
}

- (void)disconnected {
  logger.debug() << "disconnected";
}

@end
