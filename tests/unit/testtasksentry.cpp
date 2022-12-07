/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testtasksentry.h"
#include "../../src/settingsholder.h"

#include <QTextStream>

#include "../../src/tasks/sentry/tasksentry.h"
#include "helper.h"

void TestTaskSentry::envelopeWithJustAnAttachment() {
  // Envelope with just an attachment
  QByteArray envelope;
  QTextStream writer(&envelope);
  writer << "HELLo" << Qt::endl;
  auto result = TaskSentry(envelope).parseEnvelope();
  QCOMPARE(result, TaskSentry::ContentType::Unknown);
}

void TestTaskSentry::emptyEnvelope() {
  // Envelope with that"s empty
  QByteArray envelope;
  auto result = TaskSentry(envelope).parseEnvelope();
  QCOMPARE(result, TaskSentry::ContentType::Unknown);
}

void TestTaskSentry::emptyHeaderEnvelope() {
  // An just sending an empty header is not useful, but valid.
  QByteArray envelope;
  QTextStream writer(&envelope);
  writer << "{}" << Qt::endl;
  writer << Qt::endl;
  auto result = TaskSentry(envelope).parseEnvelope();
  QCOMPARE(result, TaskSentry::ContentType::Ping);
}

void TestTaskSentry::wrongOrderEnvelope() {
  // Envelope with an valid header and attachment in wrong order.
  QByteArray envelope;
  QTextStream writer(&envelope);
  writer << Qt::endl;
  writer << "\xef\xbb\xbfHello\r\n\n";
  writer << "{\"type\":\"attachment\",\"length\":10,\"content_type\":\"text/"
            "plain\",\"filename\":\"hello.txt\"}"
         << Qt::endl;
  auto result = TaskSentry(envelope).parseEnvelope();
  QCOMPARE(result, TaskSentry::ContentType::Unknown);
}

void TestTaskSentry::valid2EmptyAttachments() {
  // Envelope with 2 empty attachments:
  QByteArray envelope;
  QTextStream writer(&envelope);
  writer << "{\"event_id\":\"9ec79c33ec9942ab8353589fcb2e04dc\"}" << Qt::endl;
  writer << "{\"type\":\"attachment\",\"length\":0}" << Qt::endl;
  writer << Qt::endl;
  writer << "{\"type\":\"attachment\",\"length\":0}" << Qt::endl;
  writer << Qt::endl;
  auto result = TaskSentry(envelope).parseEnvelope();
  QCOMPARE(result, TaskSentry::ContentType::Ping);
}

void TestTaskSentry::validWithAttachments() {
  // Envelope with 2 Items, last newline omitted:
  QByteArray envelope;
  QTextStream writer(&envelope);
  writer << "{\"event_id\":\"9ec79c33ec9942ab8353589fcb2e04dc\",\"dsn\":"
            "\"https://e12d836b15bb49d7bbf99e64295d995b:@sentry.io/42\"}"
         << Qt::endl;
  writer << "{\"type\":\"attachment\",\"length\":10,\"content_type\":\"text/"
            "plain\",\"filename\":\"hello.txt\"}"
         << Qt::endl;
  writer << "\xef\xbb\xbfHello\r\n\n";
  writer << "{\"type\":\"event\",\"length\":41,\"content_type\":\"application/"
            "json\",\"filename\":\"application.log\"}"
         << Qt::endl;
  writer << "{\"message\":\"hello world\",\"level\":\"error\"}";
  auto result = TaskSentry(envelope).parseEnvelope();
  QCOMPARE(result, TaskSentry::ContentType::Ping);
}

void TestTaskSentry::validImplicitLength() {
  // Item with implicit length, terminated by newline:
  QByteArray envelope;
  QTextStream writer(&envelope);
  writer << "{\"event_id\":\"9ec79c33ec9942ab8353589fcb2e04dc\"}" << Qt::endl;
  writer << "{\"type\":\"attachment\"}" << Qt::endl;
  writer << "\xef\xbb\xbfHello\r\n\n";
  auto result = TaskSentry(envelope).parseEnvelope();
  QCOMPARE(result, TaskSentry::ContentType::Ping);
}

void TestTaskSentry::noHeaderButSession() {
  // Envelope without headers, implicit length,
  // last newline omitted, terminated by EOF:
  QByteArray envelope;
  QTextStream writer(&envelope);
  writer << "{}" << Qt::endl;
  writer << "{\"type\":\"session\"}" << Qt::endl;
  writer << "{\"started\": "
            "\"2020-02-07T14:16:00Z\",\"attrs\":{\"release\":\"sentry-test@1.0."
            "0\"}}";
  auto result = TaskSentry(envelope).parseEnvelope();
  QCOMPARE(result, TaskSentry::ContentType::Ping);
}

void TestTaskSentry::levelErrorIsFine() {
  // if the Level is == error that is not a crash.
  QByteArray envelope;
  QTextStream writer(&envelope);
  writer << "{\"event_id\":\"d7fde707\",\"timestamp\":\"2022-12-07\","
            "\"platform\":\"native\",\"level\":\"error\"}"
         << Qt::endl;
  writer << "{\"type\":\"attachment\"}" << Qt::endl;
  writer << "MMMMM" << Qt::endl;
  auto result = TaskSentry(envelope).parseEnvelope();
  QCOMPARE(result, TaskSentry::ContentType::Ping);
}

void TestTaskSentry::detectMinidump() {
  QByteArray envelope;
  QTextStream writer(&envelope);
  writer << "{}" << Qt::endl;
  writer << "{\"type\":\"minidump\",\"length\":4}" << Qt::endl;
  writer << "MDMP" << Qt::endl;
  writer << "{\"type\":\"attachment\",\"length\":12}" << Qt::endl;
  writer << "HELLO WORLD" << Qt::endl;
  auto result = TaskSentry(envelope).parseEnvelope();
  QCOMPARE(result, TaskSentry::ContentType::CrashReport);
}

void TestTaskSentry::sniffMinidump() {
  // Have a normal attachment but that contains a dump
  QByteArray envelope;
  QTextStream writer(&envelope);
  writer << "{}" << Qt::endl;
  writer << "{\"type\":\"attachment\",\"length\":12}" << Qt::endl;
  writer << "MDMP" << Qt::endl;
  auto result = TaskSentry(envelope).parseEnvelope();
  QCOMPARE(result, TaskSentry::ContentType::CrashReport);
}
void TestTaskSentry::detectAttachmentMinidump() {
  // Have an attachment, marking the minidump via attachment type
  QByteArray envelope;
  QTextStream writer(&envelope);
  writer << "{}" << Qt::endl;
  writer << "{\"type\":\"attachment\",\"attachment_type\":\"event.minidump\"}"
         << Qt::endl;
  writer << "MMMMM" << Qt::endl;
  auto result = TaskSentry(envelope).parseEnvelope();
  QCOMPARE(result, TaskSentry::ContentType::CrashReport);
}

void TestTaskSentry::detectFatalErrors() {
  // Detect fatal errors as crashreports.
  QByteArray envelope;
  QTextStream writer(&envelope);
  writer << "{\"event_id\":\"d7fde707\",\"timestamp\":\"2022-12-07\","
            "\"platform\":\"native\",\"level\":\"fatal\"}"
         << Qt::endl;
  writer << "{\"type\":\"attachment\"}" << Qt::endl;
  writer << "MMMMM" << Qt::endl;
  auto result = TaskSentry(envelope).parseEnvelope();
  QCOMPARE(result, TaskSentry::ContentType::CrashReport);
}

static TestTaskSentry s_TestTaskSentry;