/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QObject>

namespace WebExtension {
/**
 * @brief - Base Adapter for WebExtension RPC Calls
 *
 * This Class should consume JSON messages some transport via
 * BaseAdapter::onMessage. If the Current adapter has info to send to the
 * extension it will fire the BaseAdapter::onOutGoingMessage signal.
 *
 * All JSON Messages must follow the schema:
 *
 * let msg = { t: String;
 *  ... : any
 * }
 * msg.t Set's the Request Type and therefore controls
 * which callback recieves the message.
 *
 * See also BaseAdapter::RequestType
 *
 */
class BaseAdapter : public QObject {
  Q_OBJECT

 public:
  BaseAdapter(QObject* parent) : QObject(parent) {}
  /**
   * @brief Slot for the Adapter to receive Messages from the
   * WebExtension
   *
   * @param messageType - The Type of the Message
   * @param args - A Json Object Containing the arguments
   */
  void onMessage(QJsonObject message);

  /**
   * @brief Signal emitted when the Adapter has a message
   * to be sent to the WebExtension
   *
   * @param message - The Message Object
   */
  Q_SIGNAL void onOutgoingMessage(QJsonObject& message);

  struct RequestType {
    QString m_name;
    std::function<QJsonObject(const QJsonObject&)> m_callback;
  };

 protected:
  QList<RequestType> m_commands;
};

}  // namespace WebExtension
