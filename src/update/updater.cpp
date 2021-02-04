/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "updater.h"
#include "logger.h"
#include "leakdetector.h"
#include "versionapi.h"

#if defined(MVPN_WINDOWS) || defined(MVPN_MACOS)
#  include "balrog.h"
#endif

namespace {
Logger logger(LOG_NETWORKING, "Updater");
}

// static
Updater* Updater::create(QObject* parent, bool downloadAndInstall) {
#if defined(MVPN_WINDOWS) || defined(MVPN_MACOS)
  if (!downloadAndInstall) {
    return new Balrog(parent, false);
  }

  return new Balrog(parent, true);
#endif

  if (!downloadAndInstall) {
    return new VersionApi(parent);
  }

  logger.log() << "No download and install supported for this platform.";
  return nullptr;
}

Updater::Updater(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(Updater);
  logger.log() << "Updater created";
}

Updater::~Updater() {
  MVPN_COUNT_DTOR(Updater);
  logger.log() << "Updater released";
}
