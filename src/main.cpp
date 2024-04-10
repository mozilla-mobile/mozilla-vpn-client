/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandlineparser.h"
#include "leakdetector.h"
#include "stdio.h"
#ifdef MZ_WINDOWS

#  include <windows.h>

#  include <iostream>

#endif

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
#endif

  CommandLineParser clp;
  return clp.parse(argc, argv);
}
