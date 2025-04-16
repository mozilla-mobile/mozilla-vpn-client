/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macoscontroller.h"

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

  if (@available(macOS 13, *)) {
    // Create the daemon delegate object.
    NSString* appId = MacOSUtils::appId();
    Q_ASSERT(appId);
    NSString* plistName =
      QString("%1.daemon.plist").arg(QString::fromNSString(appId)).toNSString();

    m_service = [SMAppService daemonServiceWithPlistName:plistName];
  }

  m_smAppStatus = -1;
}

MacOSController::~MacOSController() {
  if (@available(macOS 13, *)) {
    SMAppService* service = static_cast<SMAppService*>(m_service);
    [service dealloc];
  }
}

void MacOSController::initialize(const Device* device, const Keys* keys) {
  Q_UNUSED(device);
  Q_UNUSED(keys);

  // For MacOS 13 and beyond, attempt to register the daemon using the
  // SMAppService interface.
  if (@available(macOS 13, *)) {
    // Poll for the service status to determine if it's been registered
    // successfully and check when the daemon should be running.
    // TODO: Can this be done asynchronously, eg: NSKeyValueObserving?
    connect(&m_regTimer, &QTimer::timeout, this,
            &MacOSController::checkServiceStatus);
    m_regTimer.start(SERVICE_REG_POLL_INTERVAL_MSEC);

    checkServiceStatus();
  } else {
    // Otherwise, for legacy Mac users, they will need to install the daemon
    // some other way. This is normally handled by the installer package.
    return LocalSocketController::initialize(device, keys);
  }
}

void MacOSController::checkServiceStatus(void) {
  if (@available(macOS 13, *)) {
    SMAppService* service = static_cast<SMAppService*>(m_service);
    SMAppServiceStatus status = [service status];
    if (status == m_smAppStatus) {
      return;
    }
    m_smAppStatus = status;

    NSError* error = nil;
    switch (status) {
      case SMAppServiceStatusNotRegistered:
        logger.debug() << "Mozilla VPN daemon not found.";
        [[fallthrough]];
      case SMAppServiceStatusNotFound:
        if ([service registerAndReturnError: & error]) {
          logger.debug() << "Mozilla VPN daemon registered successfully.";
          break;
        }
        logger.error() << "Failed to register Mozilla VPN daemon: "
                       << QString::fromNSString(error.localizedDescription);
        if (error.code == kSMErrorInvalidSignature) {
          // If the build is unsigned, continue anyways and hope for the best.
          // This is to mitigate developer pain by allowing the VPN to make use
          // of a pre-existing daemon.
          m_regTimer.stop();
          LocalSocketController::initialize(nullptr, nullptr);
        }
        break;

      case SMAppServiceStatusEnabled:
        logger.debug() << "Mozilla VPN daemon enabled.";

        // We can continue with initialization.
        // NOTE: It just so happens that the LocalSocketController doesnt use the
        // device or keys, so it's safe to pass null here.
        m_regTimer.stop();
        LocalSocketController::initialize(nullptr, nullptr);
        break;

      case SMAppServiceStatusRequiresApproval:
        logger.debug() << "Mozilla VPN daemon requires approval.";
        emit permissionRequired();
        break;
    }
  }
}
