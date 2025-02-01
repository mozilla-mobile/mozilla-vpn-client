/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "sentrysniffer.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <optional>

bool isCrashReportHeader(const QJsonObject& obj) {
  // Check 1: Test if this might be a dump attachment
  if (obj.contains("type")) {
    auto type = obj["type"].toString();
    // It's type minidump:
    if (type == "minidump") {
      return true;
    }
    if (type == "attachment") {
      // It's an attachment to an event.
      // If it's not a generic one
      // it needs to set 'attachment_type'
      if (!obj.contains("attachment_type")) {
        // This is a generic one
        return false;
      }
      auto type = obj["attachment_type"].toString();
      if (type == "event.minidump" || type == "event.applecrashreport") {
        // The attachment is a dump.
        return true;
      }
    }
    return false;
  }
  // Check 2: Check if this is a report for a crash
  if (obj.contains("level")) {
    // We might not send a minidump but still report a crash here :)
    auto level = obj["level"].toString();
    return level == "fatal";
  }
  return false;
}

SentrySniffer::SniffResult SentrySniffer::parseEnvelope(
    const QByteArrayView& data) {
  auto envelope = QString::fromUtf8(data);

  // The Sentry Envelope is a basic format
  // It is a series of JSON objects splited by lines.
  QStringList objects = envelope.split("\n");
  if (objects.empty()) {
    // This really should not happen.
    return {
        ContentType::Unknown,
    };
  }
  // The fist line is always the header:
  // a json object, containing the DSN and an event ID
  auto header = QJsonDocument::fromJson(objects.at(0).toUtf8());
  if (!header.isObject()) {
    return {ContentType::Unknown};
  }

  // Each Line here is a json object.
  // Usually in the form of
  // { headerObject type } \n
  // { {bodyObject} || raw bytes} \n

  foreach (auto content, objects) {
    auto doc = QJsonDocument::fromJson(content.toUtf8());
    if (!doc.isObject()) {
      // This might be an attachment body.
      // Sniff for minidump bytes, otherwise this is
      // fine :)
      if (content.startsWith("MDMP")) {
        return {ContentType::CrashReport};
      }
      // This is an attachment, fine to skip
      continue;
    }
    // The only thing right now, we don't want to send
    // crash-data, so we will only check for that.
    QJsonObject obj = doc.object();
    std::optional<QString> eventID = std::nullopt;
    if (obj.contains("event_id")) {
      eventID = obj["event_id"].toString();
    }
    if (isCrashReportHeader(obj)) {
      return {ContentType::CrashReport, eventID};
    }
  }
  return {ContentType::Ping};
}
