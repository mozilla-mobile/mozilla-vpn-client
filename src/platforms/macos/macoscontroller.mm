/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macoscontroller.h"

#include <QFile>

#include "constants.h"
#include "logger.h"
#include "macosutils.h"

#import <Cocoa/Cocoa.h>
#import <ServiceManagement/ServiceManagement.h>

namespace {
Logger logger("MacOSController");
}  // namespace

constexpr const int SERVICE_REG_POLL_INTERVAL_MSEC = 1000;

MacOSController::MacOSController() :
   LocalSocketController(Constants::MACOS_DAEMON_PATH) {
  m_smAppStatus = -1;

  m_regTimer.setInterval(SERVICE_REG_POLL_INTERVAL_MSEC);
  m_regTimer.setSingleShot(false);
  connect(&m_regTimer, &QTimer::timeout, this,
          &MacOSController::checkServiceEnabled);
}

NSString* MacOSController::plist() const {
  NSString* appId = MacOSUtils::appId();
  Q_ASSERT(appId);
  return [NSString stringWithFormat:@"%@.daemon.plist", appId];
}

void MacOSController::initialize(const Device* device, const Keys* keys) {
  // macOS >= 13: register the daemon using the SMAppService interface.
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
    } else if (error.code == 1) {
      // Weird edge case: When upgrading from a launchd-style daemon, the
      // background task manager gets hung up on the old daemon identifer
      // needs a kick to update its database.
      // 
      // Opening the login item settings is just such a kick so lets
      // pre-emptively prompt the user to do so if we get a permission denied
      // error when trying to register the daemon.
      //
      // Annoyingly - this is conveyed as an error code that isn't defined in
      // the SMAppService documentation.
      emit permissionRequired();
    } else {
      // Otherwise, we encountered some other error.
      logger.error() << "Unable to register Mozilla VPN daemon:"
                     << error.localizedDescription;
    }

    // Check the service status for how to proceed.
    m_smAppStatus = [service status];
    switch (m_smAppStatus) {
      case SMAppServiceStatusNotRegistered:
        logger.debug() << "Mozilla VPN daemon not registered.";
        // In the event of a registration error - try again.
        QTimer::singleShot(SERVICE_REG_POLL_INTERVAL_MSEC, this,
                           &MacOSController::registerService);
        break;

      case SMAppServiceStatusNotFound:
        logger.debug() << "Mozilla VPN daemon not found.";
        break;

      case SMAppServiceStatusEnabled:
        logger.debug() << "Mozilla VPN daemon enabled.";
        return LocalSocketController::initialize(nullptr, nullptr);

      case SMAppServiceStatusRequiresApproval:
        logger.debug() << "Mozilla VPN daemon requires approval.";
        emit permissionRequired();
        m_regTimer.start();
        break;
    }
  } else {
    // This method should only ever be called for macOS 13 and newer.
    // Alternatively - we could use SMJobBless for daemon regigstration.
    Q_UNREACHABLE();
  }
}

void MacOSController::checkServiceEnabled(void) {
  if (@available(macOS 13, *)) {
    // Create the daemon delegate object.
    SMAppService* service = [SMAppService daemonServiceWithPlistName:plist()];
    if ([service status] == SMAppServiceStatusEnabled) {
      logger.debug() << "Mozilla VPN daemon enabled.";

      // We can continue with initialization.
      // NOTE: It just so happens that the LocalSocketController doesn't use
      // the device or keys, so it's safe to pass null here.
      m_regTimer.stop();
      LocalSocketController::initialize(nullptr, nullptr);
    }
  } else {
    // This method should only ever be called for macOS 13 and newer.
    Q_UNREACHABLE();
  }
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
