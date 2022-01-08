/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "crashreporterfactory.h"

#include <qsystemdetection.h>

#ifdef Q_OS_WIN
#  include "platforms/windows/wincrashreporter.h"
#endif

using namespace std;

CrashReporterFactory::CrashReporterFactory() {}

shared_ptr<CrashReporter> CrashReporterFactory::createCrashReporter() {
#ifdef Q_OS_WIN
  return make_shared<WinCrashReporter>();
#endif
  return nullptr;
}
