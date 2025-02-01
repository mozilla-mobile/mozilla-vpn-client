/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testsentrysniffer.h"

#include <QTextStream>

#include "helper.h"
#include "sentry/sentrysniffer.h"

void TestTaskSentry::testParse_data() {
  QTest::addColumn<QByteArray>("input");
  QTest::addColumn<SentrySniffer::ContentType>("output");
  {
    QByteArray envelope;
    QTextStream writer(&envelope);
    writer << "{\"event_id\":\"d7fde707\",\"timestamp\":\"2022-12-07\","
              "\"platform\":\"native\",\"level\":\"fatal\"}"
           << Qt::endl;
    writer << "{\"type\":\"attachment\"}" << Qt::endl;
    writer << "MMMMM" << Qt::endl;
    QTest::addRow("detectFatalErrors")
        << envelope << SentrySniffer::ContentType::CrashReport;
  }
  {
    QByteArray envelope;
    QTextStream writer(&envelope);
    writer << "{}" << Qt::endl;
    writer << "{\"type\":\"attachment\",\"attachment_type\":\"event.minidump\"}"
           << Qt::endl;
    writer << "MMMMM" << Qt::endl;
    QTest::addRow("detectAttachmentMinidump")
        << envelope << SentrySniffer::ContentType::CrashReport;
  }
  {
    // Have a normal attachment but that contains a dump
    QByteArray envelope;
    QTextStream writer(&envelope);
    writer << "{}" << Qt::endl;
    writer << "{\"type\":\"attachment\",\"length\":12}" << Qt::endl;
    writer << "MDMP" << Qt::endl;
    QTest::addRow("sniffMinidump")
        << envelope << SentrySniffer::ContentType::CrashReport;
  }
  {
    QByteArray envelope;
    QTextStream writer(&envelope);
    writer << "{}" << Qt::endl;
    writer << "{\"type\":\"minidump\",\"length\":4}" << Qt::endl;
    writer << "MDMP" << Qt::endl;
    writer << "{\"type\":\"attachment\",\"length\":12}" << Qt::endl;
    writer << "HELLO WORLD" << Qt::endl;
    QTest::addRow("detectMinidump")
        << envelope << SentrySniffer::ContentType::CrashReport;
  }
  {  // if the Level is == error that is not a crash.
    QByteArray envelope;
    QTextStream writer(&envelope);
    writer << "{\"event_id\":\"d7fde707\",\"timestamp\":\"2022-12-07\","
              "\"platform\":\"native\",\"level\":\"error\"}"
           << Qt::endl;
    writer << "{\"type\":\"attachment\"}" << Qt::endl;
    writer << "MMMMM" << Qt::endl;
    QTest::addRow("levelErrorIsFine")
        << envelope << SentrySniffer::ContentType::Ping;
  }

  {
    // Envelope with just an attachment
    QByteArray envelope;
    QTextStream writer(&envelope);
    writer << "HELLo" << Qt::endl;

    QTest::addRow("envelopeWithJustAnAttachment")
        << envelope << SentrySniffer::ContentType::Unknown;
  }
  {  // Envelope with that"s empty
    QByteArray envelope;
    QTest::addRow("emptyEnvelope")
        << envelope << SentrySniffer::ContentType::Unknown;
  }

  {
    // An just sending an empty header is not useful, but valid.
    QByteArray envelope;
    QTextStream writer(&envelope);
    writer << "{}" << Qt::endl;
    writer << Qt::endl;
    QTest::addRow("emptyHeaderEnvelope")
        << envelope << SentrySniffer::ContentType::Ping;
  }

  {  // Envelope with an valid header and attach gment in wrong order.
    QByteArray envelope;
    QTextStream writer(&envelope);
    writer << Qt::endl;
    writer << "\xef\xbb\xbfHello\r\n\n";
    writer << "{\"type\":\"attachment\",\"length\":10,\"content_type\":\"text/"
              "plain\",\"filename\":\"hello.txt\"}"
           << Qt::endl;
    QTest::addRow("wrongOrderEnvelope")
        << envelope << SentrySniffer::ContentType::Unknown;
  }

  {  // Envelope with 2 empty attachments:
    QByteArray envelope;
    QTextStream writer(&envelope);
    writer << "{\"event_id\":\"9ec79c33ec9942ab8353589fcb2e04dc\"}" << Qt::endl;
    writer << "{\"type\":\"attachment\",\"length\":0}" << Qt::endl;
    writer << Qt::endl;
    writer << "{\"type\":\"attachment\",\"length\":0}" << Qt::endl;
    writer << Qt::endl;
    QTest::addRow("valid2EmptyAttachments")
        << envelope << SentrySniffer::ContentType::Ping;
  }

  {  // Envelope with 2 Items, last newline omitted:
    QByteArray envelope;
    QTextStream writer(&envelope);
    writer << "{\"event_id\":\"9ec79c33ec9942ab8353589fcb2e04dc\",\"dsn\":"
              "\"https://e12d836b15bb49d7bbf99e64295d995b:@sentry.io/42\"}"
           << Qt::endl;
    writer << "{\"type\":\"attachment\",\"length\":10,\"content_type\":\"text/"
              "plain\",\"filename\":\"hello.txt\"}"
           << Qt::endl;
    writer << "\xef\xbb\xbfHello\r\n\n";
    writer
        << "{\"type\":\"event\",\"length\":41,\"content_type\":\"application/"
           "json\",\"filename\":\"application.log\"}"
        << Qt::endl;
    writer << "{\"message\":\"hello world\",\"level\":\"error\"}";
    QTest::addRow("validWithAttachments")
        << envelope << SentrySniffer::ContentType::Ping;
  }
  {  // Item with implicit length, terminated by newline:
    QByteArray envelope;
    QTextStream writer(&envelope);
    writer << "{\"event_id\":\"9ec79c33ec9942ab8353589fcb2e04dc\"}" << Qt::endl;
    writer << "{\"type\":\"attachment\"}" << Qt::endl;
    writer << "\xef\xbb\xbfHello\r\n\n";
    QTest::addRow("validImplicitLength")
        << envelope << SentrySniffer::ContentType::Ping;
  }

  {  // Envelope without headers, implicit length,
    // last newline omitted, terminated by EOF:
    QByteArray envelope;
    QTextStream writer(&envelope);
    writer << "{}" << Qt::endl;
    writer << "{\"type\":\"session\"}" << Qt::endl;
    writer
        << "{\"started\": "
           "\"2020-02-07T14:16:00Z\",\"attrs\":{\"release\":\"sentry-test@1.0."
           "0\"}}";
    QTest::addRow("noHeaderButSession")
        << envelope << SentrySniffer::ContentType::Ping;
  }
}

void TestTaskSentry::testParse() {
  QFETCH(QByteArray, input);
  QFETCH(SentrySniffer::ContentType, output);
  QCOMPARE(SentrySniffer::parseEnvelope(input).type, output);
}

static TestTaskSentry s_TestTaskSentry;
