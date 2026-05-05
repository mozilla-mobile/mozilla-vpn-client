/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TEST_WEBEXTBRIDGE_H
#define TEST_WEBEXTBRIDGE_H

#include <QObject>
#include <QTest>

class TestWebExtBridge final : public QObject {
  Q_OBJECT

 private slots:
  void app_ping_failure();
  void app_ping_success();
  void async_connection();
  void async_disconnection();
  void fuzzy();
};

#endif
