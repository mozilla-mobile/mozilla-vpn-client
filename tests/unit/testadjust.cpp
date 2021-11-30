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
  params = AdjustFiltering::instance()->filterParameters(params, unknown);

  QFETCH(QString, output);
  QCOMPARE(params.toString(), output);

  QFETCH(QStringList, unknownParams);
  QCOMPARE(unknown, unknownParams);
}

void TestAdjust::addFields_data() {
  QTest::addColumn<QString>("input");
  QTest::addColumn<QString>("output");
  QTest::addColumn<QString>("allowField");
  QTest::addColumn<QString>("denyField");
  QTest::addColumn<QString>("denyValue");
  QTest::addColumn<QString>("mirrorField");
  QTest::addColumn<QString>("mirroringField");
  QTest::addColumn<QString>("mirrorValue");

  QTest::addRow("allow app_name, deny adid and mirror device_name to app_name")
      << QUrlQuery{{"app_name", "test1"}, {"adid", "test2"}, {"device_name", "test3"}, {"device_type", "test4"}}.toString()
      << QUrlQuery{{"app_name", "test1"}, {"adid", "default"}, {"device_name", "test1"}, {"device_type", "test4"}}.toString()
      << "app_name"
      << "adid" << "default"
      << "device_name" << "app_name" << "error";

  QTest::addRow("allow adid again, change app_name value, ")
      << QUrlQuery{{"app_name", "test1"}, {"adid", "test2"}, {"device_name", "test3"}, {"device_type", "test4"}}.toString()
      << QUrlQuery{{"app_name", "default2"}, {"adid", "test2"}, {"device_name", "error"}, {"device_type", "test2"}}.toString()
      << "adid"
      << "app_name" << "default2"
      << "device_type" << "adid" << "error";
}

void TestAdjust::addFields() {
  QFETCH(QString, input);
  QUrlQuery params(input);

  QFETCH(QString, output);

  QFETCH(QString, allowField);
  AdjustFiltering::instance()->allowField(allowField);

  QFETCH(QString, denyField);
  QFETCH(QString, denyValue);
  AdjustFiltering::instance()->denyField(denyField, denyValue);

  QFETCH(QString, mirrorField);
  QFETCH(QString, mirroringField);
  QFETCH(QString, mirrorValue);
  AdjustFiltering::instance()->mirrorField(mirrorField, {mirroringField, mirrorValue});

  QStringList unknown;
  params = AdjustFiltering::instance()->filterParameters(params, unknown);

  QCOMPARE(params.toString(), output);
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
  AdjustProxyPackageHandler* packageHandler = new AdjustProxyPackageHandler();

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
