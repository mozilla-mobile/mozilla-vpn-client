/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CRASHSERVERCLIENT_H
#define CRASHSERVERCLIENT_H

class CrashServerClient {
 public:
  CrashServerClient();
  virtual bool start(int args, char* argv[]) = 0;
};

#endif  // CRASHSERVERCLIENT_H
