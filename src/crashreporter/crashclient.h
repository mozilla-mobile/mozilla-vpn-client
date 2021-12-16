/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CRASHCLIENT_H
#define CRASHCLIENT_H

#include <memory>
#include "crashserverclient.h"

class CrashClient {
 public:
  bool start(int args, char* argv[]);
  static CrashClient& instance();

 private:
  CrashClient();
  std::shared_ptr<CrashServerClient> m_client;
};

#endif  // CRASHCLIENT_H
