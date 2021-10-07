/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testadjust.h"
#include "../../src/adjust/adjustfiltering.h"
#include "../../src/adjust/adjustproxypackagehandler.h"
#include "helper.h"

// We need this since QFETCH has a problem with a comma in the type
typedef QList<QPair<QString, QString>> PairList;
Q_DECLARE_METATYPE(PairList);

void TestAdjust::paramFiltering_data() {
  QTest::addColumn<QString>("input");
  QTest::addColumn<QString>("output");
  QTest::addColumn<QStringList>("unknownParams");

  QTest::addRow("unknown 1")
      << QUrlQuery{{"unknown", "test"}}.toString()
      << QUrlQuery{{"unknown", "test"}}.toString() << QStringList{"unknown"};

  QTest::addRow("unknown 1")
      << QUrlQuery{{"unknown", "test"}, {"unknown2", "test"}}.toString()
      << QUrlQuery{{"unknown", "test"}, {"unknown2", "test"}}.toString()
      << QStringList{"unknown", "unknown2"};

  QTest::addRow("allow")
      << QUrlQuery{{"adid", "a"}, {"idfv", "b"}, {"zone_offset", "c"}}
             .toString()
      << QUrlQuery{{"adid", "a"}, {"idfv", "b"}, {"zone_offset", "c"}}
             .toString()
      << QStringList();

  QTest::addRow("deny") << QUrlQuery{{"app_name", "a"},
                                     {"platform", "b"},
                                     {"tracker_token", "c"}}
                               .toString()
                        << QUrlQuery{{"app_name", "default"},
                                     {"platform", "default"},
                                     {"tracker_token", "xxxxx"}}
                               .toString()
                        << QStringList();

  QTest::addRow("mirror copy")
      << QUrlQuery{{"device_name", "a"}, {"device_type", "foo"}}.toString()
      << QUrlQuery{{"device_name", "foo"}, {"device_type", "foo"}}.toString()
      << QStringList();

  QTest::addRow("mirror default")
      << QUrlQuery{{"device_name", "a"}}.toString()
      << QUrlQuery{{"device_name", "dummy"}}.toString() << QStringList();

  QTest::addRow("everything together") << QUrlQuery{{"device_name", "a"},
                                                    {"foo", "bar"},
                                                    {"os_name", "os"},
                                                    {"region", "what"}}
                                              .toString()
                                       << QUrlQuery{{"device_name", "dummy"},
                                                    {"foo", "bar"},
                                                    {"os_name", "os"},
                                                    {"region", "xxxxx"}}
                                              .toString()
                                       << QStringList{"foo"};
}

void TestAdjust::paramFiltering() {
  QFETCH(QString, input);
  QUrlQuery params(input);

  QStringList unknown;
  params = AdjustFiltering::filterParameters(params, unknown);

  QFETCH(QString, output);
  QCOMPARE(params.toString(), output);

  QFETCH(QStringList, unknownParams);
  QCOMPARE(unknown, unknownParams);
}

void TestAdjust::stateMachine_data() {
  QTest::addColumn<QByteArray>("firstLine");
  QTest::addColumn<QByteArray>("headers");
  QTest::addColumn<QByteArray>("body");
  QTest::addColumn<QString>("method");
  QTest::addColumn<QString>("path");
  QTest::addColumn<QList<QPair<QString, QString>>>("parsedHeaders");

  QTest::addRow("complete")
      << QByteArray("GET /test?asdf=asdf HTTP/1.1\n")
      << QByteArray("Content-Type: application/json\n\n") << QByteArray("")
      << "GET"
      << "/test"
      << QList<QPair<QString, QString>>{{"Content-Type", "application/json"}};
  QTest::addRow("POST request") << QByteArray("POST /test HTTP/1.1\n")
                                << QByteArray("Content-Length: 9\n\n")
                                << QByteArray("test=test") << "POST"
                                << "/test" << QList<QPair<QString, QString>>{};
  QTest::addRow("exclude host header")
      << QByteArray("GET /test?asdf=asdf HTTP/1.1\n")
      << QByteArray(
             "Content-Type: application/json\nHost: "
             "localhost\nAccept: */*\n\n")
      << QByteArray("") << "GET"
      << "/test"
      << QList<QPair<QString, QString>>{{"Content-Type", "application/json"},
                                        {"Accept", "*/*"}};
}

void TestAdjust::stateMachine() {
  AdjustProxyPackageHandler* packageHandler =
      new AdjustProxyPackageHandler(this);

  QCOMPARE(packageHandler->getProcessingState(),
           AdjustProxyPackageHandler::ProcessingState::NotStarted);

  QFETCH(QByteArray, firstLine);
  packageHandler->processData(firstLine);

  QFETCH(QString, method);
  QFETCH(QString, path);
  QCOMPARE(packageHandler->getMethod(), method);
  QCOMPARE(packageHandler->getPath(), path);
  QCOMPARE(packageHandler->getProcessingState(),
           AdjustProxyPackageHandler::ProcessingState::FirstLineDone);

  QFETCH(QByteArray, headers);
  QFETCH(QByteArray, body);
  packageHandler->processData(headers);

  QFETCH(PairList, parsedHeaders);
  QCOMPARE(packageHandler->getHeaders(), parsedHeaders);
  if (body.isEmpty()) {
    QCOMPARE(packageHandler->getProcessingState(),
             AdjustProxyPackageHandler::ProcessingState::ProcessingDone);
    return;
  }
  QCOMPARE(packageHandler->getProcessingState(),
           AdjustProxyPackageHandler::ProcessingState::HeadersDone);

  packageHandler->processData(body);

  QCOMPARE(packageHandler->getProcessingState(),
           AdjustProxyPackageHandler::ProcessingState::ProcessingDone);
}

static TestAdjust s_testAdjust;
