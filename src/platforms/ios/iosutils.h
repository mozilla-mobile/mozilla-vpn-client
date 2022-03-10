/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IOSUTILS_H
#define IOSUTILS_H

#include <QString>

class IOSUtils final {
 public:
  static NSString* appId();

  static QString computerName();

  static QString IAPReceipt();

  static void shareLogs(const QString& logs);

  static int compareStrings(const QString& a, const QString& b);
};

#endif  // IOSUTILS_H
