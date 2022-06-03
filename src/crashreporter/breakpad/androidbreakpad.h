/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDBREAKPADCLIENT_H
#define ANDROIDBREAKPADCLIENT_H


#include "client/linux/handler/exception_handler.h"
#include "client/linux/handler/minidump_descriptor.h"


class AndroidBreakpadClient {

 public:

  static bool panic(const google_breakpad::MinidumpDescriptor& descriptor,
                  void* context,
                  bool succeeded);

 private:
  AndroidBreakpadClient();
  ~AndroidBreakpadClient();
};

#endif  // ANDROIDBREAKPADCLIENT_H
