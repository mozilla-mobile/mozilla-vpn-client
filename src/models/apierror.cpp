/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "apierror.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

ApiError::ApiError(const ApiError& other) {
  *this = other;
}

ApiError& ApiError::operator=(const ApiError& other) {
  if (this == &other) return *this;

  m_code = other.m_code;
  m_errnum = other.m_errnum;
  m_message = other.m_message;

  return *this;
}

bool ApiError::fromJson(const QByteArray& json) {
  QJsonDocument doc = QJsonDocument::fromJson(json);
  if (!doc.isObject()) {
    return false;
  }
  QJsonObject obj = doc.object();

  QJsonValue code = obj.value("code");
  if (!code.isDouble()) {
    return false;
  }
  QJsonValue errnum = obj.value("errno");
  if (!errnum.isDouble()) {
    return false;
  }
  QJsonValue message = obj.value("error");
  if (!message.isString()) {
    return false;
  }

  m_code = code.toInt();
  m_errnum = errnum.toInt();
  m_message = message.toString();
  return true;
}
