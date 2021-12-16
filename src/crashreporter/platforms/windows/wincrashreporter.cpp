/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wincrashreporter.h"

#include "../../crashconstants.h"
#include "wincrashdata.h"

#include <snapshot/win/process_snapshot_win.h>
#include <util/win/scoped_process_suspend.h>
#include <thread>
#include <iostream>
#include <iostream>
#include <ShlObj.h>

using namespace std;
using namespace crashpad;

constexpr auto ARG_INITIAL_DATA = "--initial-client-data";

WinCrashReporter::WinCrashReporter(QObject* parent) : CrashReporter(parent) {
  m_server = make_unique<ExceptionHandlerServer>(false);
  if (!SetProcessShutdownParameters(0x100, SHUTDOWN_NORETRY)) {
    cout << "Unable to set shutdown parameters." << endl;
  }
}

bool WinCrashReporter::start(int argc, char* argv[]) {
  m_delegate = make_shared<WinServerDelegate>();
  connect(m_delegate.get(), &WinServerDelegate::crashReported, this,
          &WinCrashReporter::crashReported);
  InitialClientData data;
  for (int i = 1; i < argc; i++) {
    if (strncmp(ARG_INITIAL_DATA, argv[i], strlen(ARG_INITIAL_DATA)) == 0) {
      cout << argv[i];
      istringstream initArg(argv[i]);
      vector<string> parts;
      string part;
      while (getline(initArg, part, '=')) {
        parts.push_back(part);
      }
      if (parts.size() > 1) {
        data.InitializeFromString(parts[1]);
      }
      break;
    }
  }
  m_server->InitializeWithInheritedDataForInitialClient(data, m_delegate.get());
  m_server->Run(m_delegate.get());
  return true;
}

std::shared_ptr<crashpad::ProcessSnapshot> WinCrashReporter::createSnapshot(
    std::shared_ptr<CrashData> data) {
  auto winData = static_pointer_cast<WinCrashData>(data);
  cout << "Creating snapshot for " << winData->getHandle() << endl;
  ScopedProcessSuspend suspend(winData->getHandle());
  Q_ASSERT(winData);
  auto snapshot = make_shared<crashpad::ProcessSnapshotWin>();
  if (!snapshot->Initialize(winData->getHandle(),
                            ProcessSuspensionState::kSuspended,
                            winData->getExceptionInfoAddress(),
                            winData->getDebugCritSectionAddress())) {
    cout << "Error creating snapsnot";
    return nullptr;
  }
  cout << "Crash report created." << endl;
  return snapshot;
}
