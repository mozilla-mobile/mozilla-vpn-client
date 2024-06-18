/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testSocks5client.h"

#include <QObject>
#include <QTest>

#include "socks5client.h"

void TestServer::testCanConnect() { qDebug() << "HELLOOO"; }

QTEST_MAIN(TestServer)
