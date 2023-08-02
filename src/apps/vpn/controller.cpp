/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "controller.h"

#include "app.h"
#include "appconstants.h"
#include "apppermission.h"
#include "captiveportal/captiveportal.h"
#include "controllerimpl.h"
#include "dnshelper.h"
#include "feature.h"
#include "frontend/navigator.h"
#include "ipaddress.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/devicemodel.h"
#include "models/keys.h"
#include "models/server.h"
#include "models/servercountrymodel.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "rfc/rfc1112.h"
#include "rfc/rfc1918.h"
#include "rfc/rfc4193.h"
#include "rfc/rfc4291.h"
#include "serveri18n.h"
#include "serverlatency.h"
#include "settingsholder.h"
#include "tasks/controlleraction/taskcontrolleraction.h"
#include "tasks/function/taskfunction.h"
#include "tasks/heartbeat/taskheartbeat.h"
#include "taskscheduler.h"
#include "tutorial/tutorial.h"

#if defined(MZ_LINUX)
#  include "platforms/linux/linuxcontroller.h"
#elif defined(MZ_MACOS) || defined(MZ_WINDOWS)
#  include "localsocketcontroller.h"
#elif defined(MZ_IOS)
#  include "platforms/ios/ioscontroller.h"
#elif defined(MZ_ANDROID)
#  include "platforms/android/androidcontroller.h"
#else
#  include "platforms/dummy/dummycontroller.h"
#endif

namespace {
Logger logger("Connection Manager");
}

Controller::Controller() {
  MZ_COUNT_CTOR(Controller);
}

Controller::~Controller() {
  MZ_COUNT_DTOR(Controller);
}

bool Controller::isVPNActive()
{
  return m_VPNActive;
}

void Controller::serializeLogs(
    std::function<void(const QString& name, const QString& logs)>&&
        a_callback) {
  std::function<void(const QString& name, const QString&)> callback =
      std::move(a_callback);

  if (!m_impl) {
    callback("Mozilla VPN backend logs", QString());
    return;
  }

  m_impl->getBackendLogs([callback](const QString& logs) {
    callback("Mozilla VPN backend logs", logs);
  });
}
