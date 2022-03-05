/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef UTILS_H
#define UTILS_H

#include <QString>

class TestUtils final {
 public:
  static QString fetchSessionCode(const QString& account);
  static QString fetchUnblockCode(const QString& account);
  static QString fetchCode(const QString& account, const QString& code);
};

#endif  // UTILS_H
