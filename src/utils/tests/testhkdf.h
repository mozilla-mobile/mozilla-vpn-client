/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QObject>
#include "testhelper.h"

class TestHKDF : public QObject, TestHelper<TestHKDF> {
  Q_OBJECT
 private:
  QByteArray fromHex(const QString& str) {
    return QByteArray::fromHex(str.toUtf8());
  };
  template<typename... Args>
  QByteArray fromHex(const QString& str, Args... args) {
    return fromHex(str) + fromHex(args...);
  }

 private slots:
  void rfc5869_data();
  void rfc5869();
};
