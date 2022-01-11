/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSCRASHCLIENT_H
#define WINDOWSCRASHCLIENT_H

#include "../../crashserverclient.h"
#include <string>

class WindowsCrashClient : public CrashServerClient {
 public:
  WindowsCrashClient();
  ~WindowsCrashClient();
  bool start(int args, char* argv[]) override;
  bool launchUploader();

 private:
  std::string m_launchPath;
};

#endif  // WINDOWSCRASHCLIENT_H
