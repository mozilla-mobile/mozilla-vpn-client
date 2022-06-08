/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "updater.h"
#include "constants.h"
#include "inspector/inspectorhandler.h"
#include "logger.h"
#include "leakdetector.h"
#include "mozillavpn.h"
#include "telemetry/gleansample.h"
#include "versionapi.h"
#include "webupdater.h"

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

  return new WebUpdater(parent);
}

Updater::Updater(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(Updater);
  connect(this, &Updater::updateRecommended, [this] {
    m_recommendedOrRequired = true;

    emit MozillaVPN::instance()->recordGleanEventWithExtraKeys(
        GleanSample::updateStep,
        {{"state",
          QVariant::fromValue(RecommendedUpdateAvailable).toString()}});
  });

  connect(this, &Updater::updateRequired, [this] {
    m_recommendedOrRequired = true;

    emit MozillaVPN::instance()->recordGleanEventWithExtraKeys(
        GleanSample::updateStep,
        {{"state", QVariant::fromValue(RequiredUpdateAvailable).toString()}});
  });
  logger.debug() << "Updater created";
}

Updater::~Updater() {
  MVPN_COUNT_DTOR(Updater);
  logger.debug() << "Updater released";
}

// static
QString Updater::appVersion() {
  if (!Constants::inProduction()) {
    return InspectorHandler::appVersionForUpdate();
  }
  return Constants::versionString();
}

// static
void Updater::updateViewShown() {
  emit MozillaVPN::instance()->recordGleanEventWithExtraKeys(
      GleanSample::updateStep,
      {{"state", QVariant::fromValue(UpdateViewShown).toString()}});
}
