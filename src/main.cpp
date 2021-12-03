/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandlineparser.h"
#include "leakdetector.h"
#include <iostream>
#include <QTimer>
#include <QCoreApplication>
#include <nebula.h>
#include <glean.h>
#include <crashreporter/crashreporterfactory.h>

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

  INIT_NEBULA;

  INIT_GLEAN;
  CommandLineParser clp;
  return clp.parse(argc, argv);
}
