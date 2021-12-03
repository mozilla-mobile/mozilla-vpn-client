/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "crashreporterapp.h"

#include <QCoreApplication>
#include <QTimer>

#include "crashreporterfactory.h"

int CrashReporterApp::main(int argc, char *argv[]){
    QCoreApplication a(argc, argv);
    QTimer::singleShot(0, &a, [](){
        auto crashreporter = CrashReporterFactory::createCrashReporter();
        crashreporter->start();
    });
    return a.exec();
}
