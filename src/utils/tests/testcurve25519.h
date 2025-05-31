/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QObject>

#include "testhelper.h"

class TestCurve25519 : public QObject, TestHelper<TestCurve25519> {
  Q_OBJECT

 private slots:
  void rfc7748vectors_data();
  void rfc7748vectors();
};
