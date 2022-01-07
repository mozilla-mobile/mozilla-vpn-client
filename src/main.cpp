/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandlineparser.h"
#include "leakdetector.h"
#include <iostream>
#include <crashreporter/crashreporterapp.h>
#include <crashreporter/crashclient.h>

#if defined MVPN_WINDOWS && defined MVPN_DEBUG
#  include <windows.h>
#endif

int main(int argc, char* argv[]) {
#ifdef MVPN_DEBUG
  LeakDetector leakDetector;
  Q_UNUSED(leakDetector);
#  ifdef MVPN_WINDOWS
  // Allocate a console to view log output in debug mode on windows
  if (AllocConsole()) {
    FILE* unusedFile;
    freopen_s(&unusedFile, "CONOUT$", "w", stdout);
    freopen_s(&unusedFile, "CONOUT$", "w", stderr);
    std::cout.clear();
    std::clog.clear();
    std::cerr.clear();
  }
#  endif

#endif
#ifdef MVPN_WINDOWS
  if (argc > 1) {
    for (int i = 1; i < argc; i++) {
      if (!strcmp("--crashreporter", argv[i])) {
        return CrashReporterApp::main(argc, argv);
      }
    }
  }
  CrashClient::instance().start(argc, argv);
#endif
  CommandLineParser clp;
  return clp.parse(argc, argv);
}
