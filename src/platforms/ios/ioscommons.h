/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IOSCOMMONS_H
#define IOSCOMMONS_H

#include <QString>

#include "theme.h"

class IOSCommons final {
 public:
  static int compareStrings(const QString& a, const QString& b);

  static void setStatusBarTextColor(Theme::StatusBarTextColor color);
  static void statusBarUpdateHack();

  static QStringList systemLanguageCodes();

  static bool verifySignature(const QByteArray& publicKey,
                              const QByteArray& content,
                              const QByteArray& signature);

  static void shareLogs(const QString& logs);
};

#endif  // IOSCOMMONS_H
