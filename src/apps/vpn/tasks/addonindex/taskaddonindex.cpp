/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskaddonindex.h"

#include "addons/manager/addonmanager.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"

namespace {
Logger logger("TaskAddonIndex");
}

TaskAddonIndex::TaskAddonIndex() : Task("TaskAddonIndex") {
  MZ_COUNT_CTOR(TaskAddonIndex);
}

TaskAddonIndex::~TaskAddonIndex() { MZ_COUNT_DTOR(TaskAddonIndex); }

void TaskAddonIndex::run() {
  // Index file
  {
    QString manifestUrl =
        QString("%1manifest.json").arg(AddonManager::addonServerAddress());
    logger.debug() << "Download manifest URL:" << manifestUrl;

    NetworkRequest* request = new NetworkRequest(this, 200);
    request->get(manifestUrl);

    connect(request, &NetworkRequest::requestFailed, this,
            [this](QNetworkReply::NetworkError error, const QByteArray&) {
              logger.error() << "Get addon index failed" << error;
              AddonManager::instance()->updateIndex(false);
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
  if (AddonManager::signatureVerificationNeeded()) {
    QString manifestSigUrl =
        QString("%1manifest.json.sig").arg(AddonManager::addonServerAddress());
    logger.debug() << "Download manifest signature URL:" << manifestSigUrl;

    NetworkRequest* request = new NetworkRequest(this, 200);
    request->get(manifestSigUrl);

    connect(request, &NetworkRequest::requestFailed, this,
            [this](QNetworkReply::NetworkError error, const QByteArray&) {
              logger.error() << "Get addon index signature failed" << error;
              AddonManager::instance()->updateIndex(false);
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

  if (AddonManager::signatureVerificationNeeded() &&
      m_indexSignData.isEmpty()) {
    return;
  }

  AddonManager::instance()->updateIndex(true, m_indexData, m_indexSignData);
  emit completed();
}
