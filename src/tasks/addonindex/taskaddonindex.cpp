/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskaddonindex.h"
#include "addonmanager.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"

namespace {
Logger logger(LOG_MAIN, "TaskAddonIndex");
}

TaskAddonIndex::TaskAddonIndex() : Task("TaskAddonIndex") {
  MVPN_COUNT_CTOR(TaskAddonIndex);
}

TaskAddonIndex::~TaskAddonIndex() { MVPN_COUNT_DTOR(TaskAddonIndex); }

void TaskAddonIndex::run() {
  NetworkRequest* request = NetworkRequest::createForGetUrl(
      this, QString("%1manifest.json").arg(Constants::addonSourceUrl()), 200);

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Get addon index failed" << error;
            emit completed();
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Get addon index completed";
            AddonManager::instance()->updateIndex(data);
            emit completed();
          });
}
