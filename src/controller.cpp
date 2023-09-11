/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "controller.h"

#include "constants.h"
#include "leakdetector.h"
#include "logger.h"

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
Logger logger("Controller");
}

Controller::Controller() { MZ_COUNT_CTOR(Controller); }

Controller::~Controller() { MZ_COUNT_DTOR(Controller); }

void Controller::initialize() {
  logger.debug() << "Initializing the controller";
  m_initialized = true;
}

bool Controller::isVPNActive() { return m_VPNActive; }

void Controller::serializeLogs(
    std::function<void(const QString& name, const QString& logs)>&&
        a_callback) {
  std::function<void(const QString& name, const QString&)> callback =
      std::move(a_callback);
}
