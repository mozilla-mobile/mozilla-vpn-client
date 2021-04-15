/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOGGER_H
#define LOGGER_H

#include <string>

// A helper class to write to STDERR logging messages.
class Logger final {
 public:
  static void log(const std::string& message);
};

#endif  // LOGGER_H
