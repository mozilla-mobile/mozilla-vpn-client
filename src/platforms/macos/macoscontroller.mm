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
  m_smAppStatus = SMAppServiceStatusNotRegistered;
}

void MacOSController::initialize(const Device* device, const Keys* keys) {
  Q_UNUSED(device);
  Q_UNUSED(keys);

  NSString* appId = MacOSUtils::appId();
  Q_ASSERT(appId);

  NSError* error = nil;
  NSString* daemonPlistName =
    QString("%1.daemon.plist").arg(QString::fromNSString(appId)).toNSString();
  SMAppService* daemon = [SMAppService daemonServiceWithPlistName:daemonPlistName];
  if (![daemon registerAndReturnError: & error]) {
    logger.error() << "Failed to register Mozilla VPN daemon: "
                   << QString::fromNSString(error.localizedDescription);
  } else {
    logger.debug() << "Mozilla VPN daemon registered successfully.";
  }
  m_smAppService = daemon;

  // Poll for the service status to determine if it's been registered
  // successfully and check when the daemon should be running.
  // TODO: Can this be done asynchronously, eg: NSKeyValueObserving?
  connect(&m_regTimer, &QTimer::timeout, this,
          &MacOSController::checkServiceStatus);
  m_regTimer.start(SERVICE_REG_POLL_INTERVAL_MSEC);
}

void MacOSController::checkServiceStatus(void) {
  SMAppService* daemon = static_cast<SMAppService*>(m_smAppService);
  SMAppServiceStatus status = [daemon status];
  if (status == m_smAppStatus) {
    return;
  }

  switch ([daemon status]) {
    case SMAppServiceStatusNotRegistered:
      logger.debug() << "Mozilla VPN daemon not registered.";
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

    case SMAppServiceStatusNotFound:
      logger.debug() << "Mozilla VPN daemon not found.";
      break;
  }
}
