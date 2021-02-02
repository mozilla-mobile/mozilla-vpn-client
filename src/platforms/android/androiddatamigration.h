
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDDATAMIGRATION_H
#define ANDROIDDATAMIGRATION_H

#include <QString>

class AndroidDataMigration final {
 public:
  static void migrate();

  // For unit-tests and internals

  static void importDeviceInfoInternal(const QByteArray& json,
                                       QString& privateKey, QString& publicKey,
                                       QString& name);
  static QByteArray importUserInfoInternal(const QByteArray& json);
  static QByteArray importServerListInternal(const QByteArray& json);

 private:
  static void importDeviceInfo();
  static void importUserInfo();
  static void importLoginToken();
  static void importServerList();
};

#endif  // ANDROIDDATAMIGRATION_H
