/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addons/manager/addonmanager.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/feature.h"
#include "networkrequest.h"
#include "taskaddonindex.h"

namespace {
Logger logger(LOG_MAIN, "TaskAddonIndex");
}

TaskAddonIndex::TaskAddonIndex() : Task("TaskAddonIndex") {
  MVPN_COUNT_CTOR(TaskAddonIndex);
}

TaskAddonIndex::~TaskAddonIndex() { MVPN_COUNT_DTOR(TaskAddonIndex); }

void TaskAddonIndex::run() {
  // Index file
  {
    NetworkRequest* request = NetworkRequest::createForGetUrl(
        this,
        QString("%1manifest.json").arg(AddonManager::addonServerAddress()),
        200);

    connect(request, &NetworkRequest::requestFailed, this,
            [this](QNetworkReply::NetworkError error, const QByteArray&) {
              logger.error() << "Get addon index failed" << error;
              emit completed();
            });

    connect(request, &NetworkRequest::requestCompleted, this,
            [this](const QByteArray& data) {
              logger.debug() << "Get addon index completed";

              m_indexData = data;
              maybeComplete();
            });
  }

  // Index file signature
  if (Feature::get(Feature::Feature_addonSignature)->isSupported()) {
    NetworkRequest* request = NetworkRequest::createForGetUrl(
        this,
        QString("%1manifest.json.sign").arg(AddonManager::addonServerAddress()),
        200);

    connect(request, &NetworkRequest::requestFailed, this,
            [this](QNetworkReply::NetworkError error, const QByteArray&) {
              logger.error() << "Get addon index signature failed" << error;
              emit completed();
            });

    connect(request, &NetworkRequest::requestCompleted, this,
            [this](const QByteArray& data) {
              logger.debug() << "Get addon index signature completed";

              m_indexSignData = data;
              maybeComplete();
            });
  }
}

void TaskAddonIndex::maybeComplete() {
  if (m_indexData.isEmpty()) {
    return;
  }

  if (Feature::get(Feature::Feature_addonSignature)->isSupported() &&
      m_indexSignData.isEmpty()) {
    return;
  }

  AddonManager::instance()->updateIndex(m_indexData, m_indexSignData);
  emit completed();
}
