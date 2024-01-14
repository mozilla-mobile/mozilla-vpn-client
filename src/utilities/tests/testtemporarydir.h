/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QObject>
#include <QTest>

class TestTemporaryDir final : public QObject {
  Q_OBJECT

 private slots:
  void basic();
  void fallback();
  void cleanupAll();
};