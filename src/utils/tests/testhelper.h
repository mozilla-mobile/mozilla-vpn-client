/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TESTHELPER_H
#define TESTHELPER_H

#include <QObject>

// Test classes should inherit this to register themselves.
class TestHelper : public QObject {
  Q_OBJECT

 public:
  static QList<const QMetaObject*> testcases();
};

#endif  // TESTHELPER_H
