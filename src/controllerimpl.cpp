/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "controllerimpl.h"

#include <QJsonObject>
#include <QJsonValue>

#include "logger.h"

namespace {
Logger logger("ControllerImpl");
}  // namespace

void ControllerImpl::emitStatusFromJson(const QJsonObject& obj) {
  QJsonValue serverIpv4Gateway = obj.value("serverIpv4Gateway");
  if (!serverIpv4Gateway.isString()) {
    logger.error() << "Unexpected serverIpv4Gateway value";
    return;
  }

  QJsonValue deviceIpv4Address = obj.value("deviceIpv4Address");
  if (!deviceIpv4Address.isString()) {
    logger.error() << "Unexpected deviceIpv4Address value";
    return;
  }

  QJsonValue txBytes = obj.value("txBytes");
  if (!txBytes.isDouble()) {
    logger.error() << "Unexpected txBytes value";
    return;
  }

  QJsonValue rxBytes = obj.value("rxBytes");
  if (!rxBytes.isDouble()) {
    logger.error() << "Unexpected rxBytes value";
    return;
  }

  emit statusUpdated(serverIpv4Gateway.toString(), deviceIpv4Address.toString(),
                     txBytes.toDouble(), rxBytes.toDouble());
}
