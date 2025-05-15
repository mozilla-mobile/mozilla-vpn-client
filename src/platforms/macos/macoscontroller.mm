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

  m_regTimer.setInterval(SERVICE_REG_POLL_INTERVAL_MSEC);
  m_regTimer.setSingleShot(false);
  connect(&m_regTimer, &QTimer::timeout, this,
          &MacOSController::checkServiceEnabled);
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
    switch ([service status]) {
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
