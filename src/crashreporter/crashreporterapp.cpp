/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "crashreporterapp.h"

#include <QApplication>
#include <QTimer>
#include <iostream>

#include "crashreporterfactory.h"

int CrashReporterApp::main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  auto crashreporter = CrashReporterFactory::createCrashReporter();
  QTimer::singleShot(0, &a, [crashreporter, argc, argv]() {
    crashreporter->start(argc, argv);
  });
  return a.exec();
}
