/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "updater.h"
#include "constants.h"
#include "inspector/inspectorwebsocketconnection.h"
#include "logger.h"
#include "leakdetector.h"
#include "versionapi.h"

#ifdef MVPN_BALROG
#  include "balrog.h"
#endif

namespace {
Logger logger(LOG_NETWORKING, "Updater");
}

// static
Updater* Updater::create(QObject* parent, bool downloadAndInstall) {
#ifdef MVPN_BALROG
  if (!downloadAndInstall) {
    return new Balrog(parent, false);
  }

  return new Balrog(parent, true);
#endif

  if (!downloadAndInstall) {
    return new VersionApi(parent);
  }

  logger.warning() << "No download and install supported for this platform.";
  return nullptr;
}

Updater::Updater(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(Updater);
  connect(this, &Updater::updateRecommended,
          [this] { m_recommendedOrRequired = true; });
  connect(this, &Updater::updateRequired,
          [this] { m_recommendedOrRequired = true; });
  logger.debug() << "Updater created";
}

Updater::~Updater() {
  MVPN_COUNT_DTOR(Updater);
  logger.debug() << "Updater released";
}

// static
QString Updater::appVersion() {
  if (!Constants::inProduction()) {
    return InspectorWebSocketConnection::appVersionForUpdate();
  }
  return APP_VERSION;
}
