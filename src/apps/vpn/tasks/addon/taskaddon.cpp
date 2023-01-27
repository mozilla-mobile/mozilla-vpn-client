/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskaddon.h"

#include "addons/manager/addonmanager.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"

namespace {
Logger logger("TaskAddon");
}

TaskAddon::TaskAddon(const QString& addonId, const QByteArray& sha256)
    : Task("TaskAddon"), m_addonId(addonId), m_sha256(sha256) {
  MZ_COUNT_CTOR(TaskAddon);
}

TaskAddon::~TaskAddon() { MZ_COUNT_DTOR(TaskAddon); }

void TaskAddon::run() {
  NetworkRequest* request = new NetworkRequest(this, 200);
  request->get(
      QString("%1%2.rcc").arg(AddonManager::addonServerAddress(), m_addonId));

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Get addon failed" << error;
            emit completed();
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Get addon completed";
            AddonManager::instance()->storeAndLoadAddon(data, m_addonId,
                                                        m_sha256);
            emit completed();
          });
}
