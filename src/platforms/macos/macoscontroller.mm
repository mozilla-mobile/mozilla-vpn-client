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
  m_smAppStatus = -1;

  m_regTimer.setInterval(SERVICE_REG_POLL_INTERVAL_MSEC);
  m_regTimer.setSingleShot(false);
  connect(&m_regTimer, &QTimer::timeout, this,
          &MacOSController::checkServiceEnabled);

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

QString MacOSController::plistName() const {
  return MacOSUtils::appId() + ".daemon.plist";
}

void MacOSController::initialize(const Device* device, const Keys* keys) {
  // For MacOS 13 and beyond, attempt to register the daemon using the
  // SMAppService interface.
  if (@available(macOS 13, *)) {
    SMAppService* service =
        [SMAppService daemonServiceWithPlistName:plistName().toNSString()];

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
      return LocalSocketController::initialize(device, keys);
    } else {
      // Otherwise, we encountered some other error. Most likely the user
      // needs to approve the daemon to run. Which will be handled below.
      logger.error() << "Unable to register Mozilla VPN daemon:"
                     << error.localizedDescription;
    }

    // Check the service status for how to proceed.
    m_smAppStatus = [service status];
    switch (m_smAppStatus) {
      case SMAppServiceStatusNotRegistered:
        logger.debug() << "Mozilla VPN daemon not registered.";
        break;

      case SMAppServiceStatusNotFound:
        logger.debug() << "Mozilla VPN daemon not found.";
        break;

      case SMAppServiceStatusEnabled:
        logger.debug() << "Mozilla VPN daemon enabled.";
        return LocalSocketController::initialize(device, keys);

      case SMAppServiceStatusRequiresApproval:
        logger.debug() << "Mozilla VPN daemon requires approval.";
        emit permissionRequired();
        m_regTimer.start();
        break;
    }
  } else {
    // Otherwise, for legacy Mac users, they will need to install the daemon
    // some other way. This is normally handled by the installer package.
    return LocalSocketController::initialize(device, keys);
  }
}

void MacOSController::checkServiceEnabled(void) {
  if (@available(macOS 13, *)) {
    // Create the daemon delegate object.
    SMAppService* service =
        [SMAppService daemonServiceWithPlistName:plistName().toNSString()];
    if ([service status] == SMAppServiceStatusEnabled) {
      logger.debug() << "Mozilla VPN daemon enabled.";

      // We can continue with initialization.
      // NOTE: It just so happens that the LocalSocketController doesn't use
      // the device or keys, so it's safe to pass null here.
      m_regTimer.stop();
      LocalSocketController::initialize(nullptr, nullptr);
    }
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
  auto conn = static_cast<NSXPCConnection*>(m_connection);
  [[conn remoteObjectProxy] getStatus];
}

void MacOSController::getBackendLogs(
    std::function<void(const QString&)>&& callback) {
  // If the daemon is connected - use the LocalSocketController to fetch logs.
  if (m_daemonState == eReady) {
    LocalSocketController::getBackendLogs(std::move(callback));
    return;
  }

  // Otherwise, try our best to scrape the logs directly off disk.
  QFile logfile("/var/log/mozillavpn/mozillavpn.log");
  if (!logfile.open(QIODeviceBase::ReadOnly | QIODeviceBase::Text)) {
    callback(QString("Failed to open backend logs"));
  } else {
    callback(logfile.readAll());
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
  //QMetaObject::invokeMethod(self.parent, "connected",
  //                          Q_ARG(QString, QString::fromNSString(pubkey)));
}

- (void)disconnected {
  //QMetaObject::invokeMethod(self.parent, "disconnected");
}

- (void)status:(NSString*)status {
  QByteArray jsBlob = QString::fromNSString(status).toUtf8();
  QJsonObject obj = QJsonDocument::fromJson(jsBlob).object();

  QString serverIpv4Gateway = obj.value("serverIpv4Gateway").toString();
  QString deviceIpv4Address = obj.value("deviceIpv4Address").toString();
  QMetaObject::invokeMethod(self.parent, "statusUpdated",
                            Q_ARG(QString, serverIpv4Gateway),
                            Q_ARG(QString, deviceIpv4Address),
                            Q_ARG(quint64, obj.value("txBytes").toInteger()),
                            Q_ARG(quint64, obj.value("rxBytes").toInteger()));
}

@end
