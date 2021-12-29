/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifdef MVPN_WINDOWS
#  include <WinSock2.h>
// clang-format off
#  pragma include_alias(<sys/time.h>, <time.h>)
// clang-format on
#endif

#include "commandlineparser.h"
#include "leakdetector.h"
#include <iostream>
#include <QTimer>
#include <QCoreApplication>
#include <nebula.h>
#include <glean.h>
#include <thread>
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
  /* if (AllocConsole()) {
    FILE* unusedFile;
    freopen_s(&unusedFile, "CONOUT$", "w", stdout);
    freopen_s(&unusedFile, "CONOUT$", "w", stderr);
    std::cout.clear();
    std::clog.clear();
    std::cerr.clear();

}*/
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
  if (!CrashClient::instance().start(argc, argv)) {
    std::cout << "CrashClient returned false." << std::endl;
  }

#endif
  CommandLineParser clp;
  std::thread t([]() {
    Sleep(2000);
    // abort();
    // throw new std::exception();
    void* p = malloc(64);
    free(p);
    free(p);
  });
  return clp.parse(argc, argv);
}
