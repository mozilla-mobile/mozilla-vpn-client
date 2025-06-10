/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandlineparser.h"

#include <QCoreApplication>

#include <CoreFoundation/CoreFoundation.h>

#include "leakdetector.h"
#include "stdio.h"

constexpr const char* MACOS_DAEMON_DEFAULT_COMMAND = "macosdaemon";

static QString getBundleVersion() {
  CFBundleRef bundle = CFBundleGetMainBundle();
  if (!bundle) {
    return "1.2.3";
  }

  CFStringRef key = QStringLiteral("CFBundleShortVersionString").toCFString();
  CFTypeRef value = CFBundleGetValueForInfoDictionaryKey(bundle, key);
  CFRelease(key);

  return QString::fromCFString(static_cast<CFStringRef>(value));
}

Q_DECL_EXPORT int main(int argc, char* argv[]) {
#ifdef MZ_DEBUG
  LeakDetector leakDetector;
  Q_UNUSED(leakDetector);
#endif

  QCoreApplication::setApplicationName("Mozilla VPN Daemon");
  QCoreApplication::setOrganizationName("Mozilla");
  QCoreApplication::setApplicationVersion(getBundleVersion());

  CommandLineParser clp;
  return clp.parse(argc, argv, MACOS_DAEMON_DEFAULT_COMMAND);
}
