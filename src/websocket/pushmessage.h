/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PUSHMESSAGE_H
#define PUSHMESSAGE_H

#include <QJsonObject>
#include <QObject>

class PushMessage final : public QObject {
  Q_OBJECT
 public:
  ~PushMessage();

  PushMessage(const QString& message);

  bool executeAction();

  const QString& type() const { return m_messageType; }

  static void registerPushMessageType(
      const QString& messageType, bool (*callback)(const QJsonObject& payload));

 private:
  QString m_messageType;
  QJsonObject m_messagePayload;
};

#endif  // PUSHMESSAGE_H
