/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QObject>

#include "testhelper.h"

class TestChaCha20Poly : public QObject, TestHelper<TestChaCha20Poly> {
  Q_OBJECT

 private slots:
  void rfc7539_data();
  void rfc7539();
};
