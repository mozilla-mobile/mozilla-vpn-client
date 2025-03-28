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

// Obj-C helper object to hold the service.
@interface MacOSDaemonDelegate : NSObject
@property(assign) SMAppService* service;
@end

@implementation MacOSDaemonDelegate {
  MacOSController* m_controller;
}

- (id)initWithObjectAndPlist:(MacOSController*)controller
                   plistName:(NSString*)name{
  self = [super init];
  if (self) {
    self.service = [SMAppService daemonServiceWithPlistName:name];
  }
  return self;
}

- (void)dealloc {
  [super dealloc];
}

@end

constexpr const int SERVICE_REG_POLL_INTERVAL_MSEC = 1000;

MacOSController::MacOSController() :
   LocalSocketController(Constants::MACOS_DAEMON_PATH) {

  // Create the daemon delegate object.
  NSString* appId = MacOSUtils::appId();
  Q_ASSERT(appId);
  NSString* plistName =
    QString("%1.daemon.plist").arg(QString::fromNSString(appId)).toNSString();

  m_delegate = [[MacOSDaemonDelegate alloc] initWithObjectAndPlist:this
                                                         plistName:plistName];

  m_smAppStatus = SMAppServiceStatusNotFound;
}

MacOSController::~MacOSController() {
  MacOSDaemonDelegate* delegate = static_cast<MacOSDaemonDelegate*>(m_delegate);
  [delegate dealloc];
}

void MacOSController::initialize(const Device* device, const Keys* keys) {
  Q_UNUSED(device);
  Q_UNUSED(keys);

  // Poll for the service status to determine if it's been registered
  // successfully and check when the daemon should be running.
  // TODO: Can this be done asynchronously, eg: NSKeyValueObserving?
  connect(&m_regTimer, &QTimer::timeout, this,
          &MacOSController::checkServiceStatus);
  m_regTimer.start(SERVICE_REG_POLL_INTERVAL_MSEC);

  checkServiceStatus();
}

void MacOSController::checkServiceStatus(void) {
  MacOSDaemonDelegate* delegate = static_cast<MacOSDaemonDelegate*>(m_delegate);
  SMAppServiceStatus status = [delegate.service status];
  if (status == m_smAppStatus) {
    return;
  }
  m_smAppStatus = status;

  NSError* error = nil;
  switch (status) {
    case SMAppServiceStatusNotRegistered:
      if (![delegate.service registerAndReturnError: & error]) {
        logger.error() << "Failed to register Mozilla VPN daemon: "
                      << QString::fromNSString(error.localizedDescription);
      } else {
        logger.debug() << "Mozilla VPN daemon registered successfully.";
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

    case SMAppServiceStatusNotFound:
      logger.debug() << "Mozilla VPN daemon not found.";
      break;
  }
}
