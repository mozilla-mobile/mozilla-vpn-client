/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "logger.h"

// static
void Logger::log(const std::string& message) {
  fwrite(message.c_str(), sizeof(char), message.length(), stderr);
  fwrite("\n", sizeof(char), 1, stderr);
}
