/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "pushmessage.h"

#include <QJsonDocument>
#include <QJsonValue>
#include <QMap>

#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("PushMessage");

QMap<QString, bool (*)(const QJsonObject&)> s_pushMessageTypes;
}  // namespace

/**
 * @brief Abstraction over a message sent from the push server.
 *
 * Messages are expected to be a valid JSON with the `type` and (optional)
 * `payload` attributes. Example:
 * ```json
 * {
 *   "type": "A VALID TYPE",
 *   "payload": {
 *     "something": "here",
 *     "as": "payload"
 *   }
 * }
 * ```
 *
 * The `type` can be one of a set of known possibilities. See
 * `PushMessage::registerPushMessageType`, to register new types.
 *
 * The payload format will depend on the `type` of the message and may be
 * omited. When present,  it will contain information necessary for executing
 * whatever action is according to the message `type`.
 */
PushMessage::PushMessage(const QString& message) {
  MZ_COUNT_CTOR(PushMessage);

  QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());

  QJsonObject obj = doc.object();

  m_messagePayload = obj["payload"].toObject();
  m_messageType = obj["type"].toString();
}

PushMessage::~PushMessage() { MZ_COUNT_DTOR(PushMessage); }

/**
 * @brief Executes the action related to the given message.
 *
 * If the message is malformatted or of unknown type, this is a no-op.
 *
 * The `onExecutionCompleted` signal is emited this function is done.
 * If nothing was done e.g. if the message was unknown, it will emit a `false`
 * value.
 */
bool PushMessage::executeAction() {
  if (!s_pushMessageTypes.contains(m_messageType)) {
    logger.debug()
        << "Attempted to execute action for unknown message. Ignoring.";
    return false;
  }

  return s_pushMessageTypes.value(m_messageType)(m_messagePayload);
}

// static
void PushMessage::registerPushMessageType(
    const QString& messageType, bool (*callback)(const QJsonObject& payload)) {
  s_pushMessageTypes.insert(messageType, callback);
}
