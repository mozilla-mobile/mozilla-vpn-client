/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wincrashreporter.h"
#include "../../crashconstants.h"
#include <base/logging.h>

#include <iostream>

using namespace std;
using namespace crashpad;

constexpr auto ARG_INITIAL_DATA = "--initial-client-data";

WinCrashReporter::WinCrashReporter(QObject* parent) : CrashReporter(parent) {
  m_server = make_unique<ExceptionHandlerServer>(false);
  logging::LoggingSettings settings;
  settings.logging_dest = logging::LOG_TO_STDERR || logging::LOG_TO_SYSTEM_DEBUG_LOG;
  logging::InitLogging(settings);
}

bool WinCrashReporter::start(int argc, char* argv[]) {
  for (int i = 1; i < argc; i++) {
    string str = argv[i];
    if (str.rfind(ARG_INITIAL_DATA, 0) == 0) {
      m_initialData.InitializeFromString(str);
      cout << "Found intial args.";
    }
  }
  assert(m_initialData.IsValid());

  WinServerDelegate* delegate = new WinServerDelegate();
  connect(delegate, &WinServerDelegate::crashReported, this,
          &WinCrashReporter::crashReported);
  // run takes ownership of the delegate pointer
  m_server->InitializeWithInheritedDataForInitialClient(m_initialData,
                                                        delegate);
  return true;
}
