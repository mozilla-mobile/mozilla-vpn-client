/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TEST_WEBEXTHANDLER_H
#define TEST_WEBEXTHANDLER_H

#include <QObject>
#include <QTest>

class TestWebExtHandler final : public QObject {
  Q_OBJECT

 private slots:
  void bridge_ping();
  void proc_info();
};

#endif
