/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QCoreApplication>

#include "commandlineparser.h"
#include "leakdetector.h"
#include "platforms/windows/windowsutils.h"
#include "stdio.h"
#include "buildinfo.h"

#ifdef MZ_WINDOWS
#  include <windows.h>

#  include <iostream>

#  include "platforms/windows/windowsutils.h"
#endif

constexpr const char* CLP_DEFAULT_COMMAND = "ui";

Q_DECL_EXPORT int main(int argc, char* argv[]) {
#ifdef MZ_DEBUG
  LeakDetector leakDetector;
  Q_UNUSED(leakDetector);
#endif

#ifdef MZ_WINDOWS
  if (AttachConsole(ATTACH_PARENT_PROCESS) != 0) {
    FILE* unusedFile;
    // Swap to the new out/err streams
    freopen_s(&unusedFile, "CONOUT$", "w", stdout);
    freopen_s(&unusedFile, "CONOUT$", "w", stderr);
    std::cout.clear();
    std::clog.clear();
    std::cerr.clear();
  }
  WindowsUtils::lockDownDLLSearchPath();
#endif

  QCoreApplication::setApplicationName("Mozilla VPN");
  QCoreApplication::setOrganizationName("Mozilla");
  QCoreApplication::setApplicationVersion(BuildInfo::version);

  CommandLineParser clp;
  return clp.parse(argc, argv, CLP_DEFAULT_COMMAND);
}
