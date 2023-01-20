/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSUTILS_H
#define WINDOWSUTILS_H

#include <QString>

class WindowsUtils final {
 public:
  static QString getErrorMessage();
  static void windowsLog(const QString& msg);

  // Returns the major version of Windows
  static QString windowsVersion();
};

#endif  // WINDOWSUTILS_H
