/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandcrashreporter.h"
#include "commandlineparser.h"
#include "leakdetector.h"

#include <crashreporter/crashreporterapp.h>

CommandCrashReporter::CommandCrashReporter(QObject* parent)
    : Command(parent, "crashreporter", "Starts the crash reporter.") {
  MVPN_COUNT_CTOR(CommandCrashReporter);
}

CommandCrashReporter::~CommandCrashReporter() {
  MVPN_COUNT_DTOR(CommandCrashReporter);
}

int CommandCrashReporter::run(QStringList& tokens) {
  return CrashReporterApp::main(CommandLineParser::argc(),
                                CommandLineParser::argv());
}

static Command::RegistrationProxy<CommandCrashReporter> s_commandCrashReporter;
