/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "appconstants.h"

#include "settingsholder.h"

namespace {
QString s_stagingServerAddress = "";
}  // namespace

QString AppConstants::apiBaseUrl() {
  if (Constants::inProduction()) {
    return AppConstants::API_PRODUCTION_URL;
  }

  return AppConstants::getStagingServerAddress();
}

const QString& AppConstants::getStagingServerAddress() {
  return s_stagingServerAddress;
}

void AppConstants::setStaging() {
  Constants::setStaging();
  s_stagingServerAddress = SettingsHolder::instance()->stagingServerAddress();
  Q_ASSERT(!s_stagingServerAddress.isEmpty());
}
