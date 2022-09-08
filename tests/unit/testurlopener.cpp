/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testurlopener.h"
#include "../../src/urlopener.h"

void TestUrlOpener::urlQueryReplacement_data() {
  QTest::addColumn<QUrl>("input");
  QTest::addColumn<QUrl>("output");

  QTest::addRow("Empty string") << QUrl() << QUrl();
  QTest::addRow("No query param")
      << QUrl("http://example.com") << QUrl("http://example.com");
  QTest::addRow("No replacement query param")
      << QUrl("http://example.com?a=b&c=42")
      << QUrl("http://example.com?a=b&c=42");
  QTest::addRow("Replacement query param")
      << QUrl(
             "http://"
             "example.com?a=__VPN_VERSION__&b=__VPN_BUILDNUMBER__&c=__VPN_OS__&"
             "d=__"
             "VPN_PLATFORM__&e=__VPN_ARCH__&f=__VPN_GRAPHICSAPI__")
      << QUrl(QString("http://example.com?a=%1&b=%2&c=%3&d=%4&e=%5&f=%6")
                  .arg(Env::versionString())
                  .arg(Env::buildNumber())
                  .arg(Env::osVersion())
                  .arg(Env::platform())
                  .arg(Env::architecture())
                  .arg(MozillaVPN::graphicsApi()));
}

void TestUrlOpener::urlQueryReplacement() {
  QFETCH(QUrl, input);
  QFETCH(QUrl, output);
  QCOMPARE(UrlOpener::replaceUrlParams(input), output);
}

static TestUrlOpener s_testUrlOpener;
