/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidbreakpad.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "settingsholder.h"
#include "qmlengineholder.h"
#include "jni.h"

#include <QJniObject>
#include <QJniEnvironment>

#include "client/linux/handler/exception_handler.h"
#include "client/linux/handler/minidump_descriptor.h"

namespace {
AndroidBreakpadClient* s_instance = nullptr;
Logger logger(LOG_ANDROID, "AndroidBreakpadClient");
}  // namespace

// static
bool AndroidBreakpadClient::panic(const google_breakpad::MinidumpDescriptor& descriptor,
                  void* context,
                  bool succeeded) {
    QJniObject::callStaticMethod<void>("org/mozilla/firefox/vpn/qt/VPNCrashReporterUtil",
                                        "nativePanic", 
                                        "(Ljava/lang/String;)V",
                                        QJniObject::fromString(descriptor.path()).object());    
   return succeeded;
}

