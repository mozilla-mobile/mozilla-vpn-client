/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "controllerimpl.h"

#include "logger.h"

namespace {
Logger logger("ControllerImpl");
}  // namespace

void ControllerImpl::getBackendLogs(QIODevice* device) {
  QString name = metaObject()->className();
  QString reply = QString("Backend logs are not supported with %1").arg(name);
  device->write(reply.toUtf8());
  device->close();
}
