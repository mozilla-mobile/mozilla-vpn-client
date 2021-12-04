/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINCRASHREPORTER_H
#define WINCRASHREPORTER_H

#include "../../crashreporter.h"
#include "util/win/initial_client_data.h"
#include "winserverdelegate.h"

#include <util/win/exception_handler_server.h>

class WinCrashReporter : public CrashReporter {
 public:
  explicit WinCrashReporter(QObject* parent = nullptr);
  bool start(int argc, char* argv[]) override;

 private:
  std::unique_ptr<crashpad::ExceptionHandlerServer> m_server;
  std::weak_ptr<WinServerDelegate> m_delegate;
  crashpad::InitialClientData m_initialData;
};

#endif  // WINCRASHREPORTER_H
