/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "pushmessage.h"

#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "controller.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/keys.h"
#include "mozillavpn.h"

namespace {
Logger logger("PushMessage");
}  // namespace

/**
 * @brief Abstraction over a message sent from the push server.
 *
 * Messages are expected to be a valid JSON with the `type` and (optional)
 * `payload` attributes. Example:
 * ```json
 * {
 *   "type": "DEVICE_DELETED",
 *   "payload": {
 *     "publicKey": "e95cfedd-119b-4ef1-aa51-46f9427e2337"
 *   }
 * }
 * ```
 *
 * The `type` can be one of a set of known possibilities. See
 * `PushMessage::messageTypeFromString`, for a comprehensive list of known
 * message types.
 *
 * The payload format will depend on the `type` of the message and may be
 * omiited. When present,  it will contain information necessary for executing
 * whatever action is according to the message `type`.
 */
PushMessage::PushMessage(const QString& message)
    : m_messageType(MessageType_UnknownMessage),
      m_messagePayload(QJsonObject()) {
  MZ_COUNT_CTOR(PushMessage);
  parseMessage(message);
}

PushMessage::~PushMessage() { MZ_COUNT_DTOR(PushMessage); }

/**
 * @brief Parses messages send from the push server.
 *
 * If the message is wrongly formatted or of an unknown type,
 * this function will not error.
 */
void PushMessage::parseMessage(const QString& message) {
  QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());

  QJsonObject obj = doc.object();

  m_messagePayload = obj["payload"].toObject();

  QString messageType = obj["type"].toString();
  m_messageType = messageTypeFromString(messageType);
}

/**
 * @brief Infers the message type from the `type` extracted from the given JSON
 * string.
 *
 * Note: These should be in sync with the type sent by the push server.
 * See:
 * https://github.com/mozilla-services/guardian-website/blob/7da6d52b334e35bd858bd22c7add5cde7ced9ef2/server/websocket/messages.ts
 */
// static
PushMessage::MessageType PushMessage::messageTypeFromString(
    const QString& str) {
  if (str == "DEVICE_DELETED") return MessageType_DeviceDeleted;
#ifdef UNIT_TEST
  if (str == "TEST_MESSAGE") return MessageType_TestMessage;
#endif
  logger.debug() << "Unknown message type" << str;
  return MessageType_UnknownMessage;
}

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
  switch (m_messageType) {
    case MessageType_DeviceDeleted:
      logger.debug() << "Executing action for 'DeviceDeleted'";
      return handleDeviceDeleted(m_messagePayload);

#ifdef UNIT_TEST
    case MessageType_TestMessage:
      logger.debug() << "Executing action for 'TestMessage'";
      return true;
#endif

    default:
      logger.debug()
          << "Attempted to execute action for unknown message. Ignoring.";
      return false;
  }
}

/**
 * @brief Handler for the `MessageType_DeviceDeleted` message type.
 *
 * Expected payload:
 *
 * ```json
 * {
 *    "publicKey": "<device public key>",
 * }
 * ```
 *
 * If the current device was the one that got deleted, user will be logged out.
 * Otherwise, deleted device will be deleted from the settings.
 */
// static
bool PushMessage::handleDeviceDeleted(const QJsonObject& payload) {
  const QString& publicKey = payload["publicKey"].toString();
  logger.debug() << publicKey;
  if (publicKey.isEmpty()) {
    logger.error()
        << "Malformed message payload for DeviceDeleted message. Ignoring.";
    return false;
  }

  MozillaVPN* vpn = MozillaVPN::instance();
  if (vpn->keys()->publicKey() == publicKey) {
    logger.info() << "Current device has been deleted from this subscription.";
    vpn->reset(true);
    return true;
  }

  MozillaVPN::instance()->deviceRemoved(publicKey, "PushMessage");
  return true;
}
