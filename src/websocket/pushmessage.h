/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PUSHMESSAGE_H
#define PUSHMESSAGE_H

#include <QVariant>
#include <QObject>
#include <QJsonObject>

class PushMessage final : public QObject {
  Q_OBJECT
 public:
  ~PushMessage();

  PushMessage(const QString& message);

  bool executeAction();

  enum MessageType {
    MessageType_DeviceDeleted,
#ifdef UNIT_TEST
    MessageType_TestMessage,
#endif
    MessageType_UnknownMessage
  };
  Q_ENUM(MessageType)

  MessageType type() const { return m_messageType; }

 private:
  static MessageType messageTypeFromString(const QString& str);
  void parseMessage(const QString& message);

  // Action handlers
  static bool handleDeviceDeleted(const QJsonObject& payload);

 private:
  MessageType m_messageType;
  QJsonObject m_messagePayload;
};

#endif  // PUSHMESSAGE_H
