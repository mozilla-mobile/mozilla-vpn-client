/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "baseadapter.h"

#include <QJsonObject>

void WebExtension::BaseAdapter::onMessage(QJsonObject message) {
  if (!message.contains("t")) {
    sendInvalidRequest();
    return;
  }
  if (!message["t"].isString()) {
    sendInvalidRequest();
    return;
  }

  QString typeName = message["t"].toString();
  for (const RequestType& type : m_commands) {
    if (typeName == type.m_name) {
      QJsonObject responseObj = type.m_callback(message);
      responseObj["t"] = typeName;
      emit onOutgoingMessage(responseObj);
      return;
    }
  }
  sendError("Command not found");
}

void WebExtension::BaseAdapter::sendInvalidRequest() {
  QJsonObject responseObj;
  responseObj["t"] = "invalidRequest";
  emit onOutgoingMessage(responseObj);
}

void WebExtension::BaseAdapter::sendError(const QString& msg) {
  QJsonObject responseObj;
  responseObj["t"] = "error";
  responseObj["msg"] = msg;
  emit onOutgoingMessage(responseObj);
}
