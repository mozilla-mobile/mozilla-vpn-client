/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testurlopener.h"

#include "env.h"
#include "urlopener.h"

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
             "example.com?a=__MZ_VERSION__&b=__MZ_BUILDNUMBER__&c=__MZ_OS__&"
             "d=__"
             "MZ_PLATFORM__&e=__MZ_ARCH__")
      << QUrl(QString("http://example.com?a=%1&b=%2&c=%3&d=%4&e=%5")
                  .arg(Env::versionString(), Env::buildNumber(),
                       Env::osVersion(), Env::platform(), Env::architecture()));
  QTest::addRow("Replacement query param (legacy)")
      << QUrl(
             "http://"
             "example.com?a=__VPN_VERSION__&b=__VPN_BUILDNUMBER__&c=__VPN_OS__&"
             "d=__"
             "VPN_PLATFORM__&e=__VPN_ARCH__")
      << QUrl(QString("http://example.com?a=%1&b=%2&c=%3&d=%4&e=%5")
                  .arg(Env::versionString(), Env::buildNumber(),
                       Env::osVersion(), Env::platform(), Env::architecture()));
}

void TestUrlOpener::urlQueryReplacement() {
  QFETCH(QUrl, input);
  QFETCH(QUrl, output);
  QCOMPARE(UrlOpener::replaceUrlParams(input), output);
}

void TestUrlOpener::urlLabel() {
  UrlOpener* uo = UrlOpener::instance();
  QVERIFY(!!uo);

  uo->registerUrlLabel("aa", []() -> QString { return "http://foo.bar"; });
  uo->setStealUrls();
  uo->openUrlLabel("aa");
  QCOMPARE(uo->lastUrl(), "http://foo.bar");
}

static TestUrlOpener s_testUrlOpener;
