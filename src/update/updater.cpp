/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "updater.h"

#include "constants.h"
#include "glean/generated/metrics.h"
#include "logger.h"
#include "mozillavpn.h"
#include "utils/leakdetector/leakdetector.h"
#include "versionapi.h"
#include "webupdater.h"

#ifdef MVPN_BALROG
#  include "balrog.h"
#endif

namespace {
Logger logger("Updater");
}

// static
Updater* Updater::create(
    QObject* parent, bool downloadAndInstall,
    ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy) {
#ifdef MVPN_BALROG
  return new Balrog(parent, downloadAndInstall, errorPropagationPolicy);
#endif

  Q_UNUSED(errorPropagationPolicy);

  if (!downloadAndInstall) {
    return new VersionApi(parent);
  }

  return new WebUpdater(parent);
}

Updater::Updater(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(Updater);
  connect(this, &Updater::updateRecommended, [this] {
    m_recommendedOrRequired = true;

    mozilla::glean::sample::update_step.record(
        mozilla::glean::sample::UpdateStepExtra{
            ._state =
                QVariant::fromValue(RecommendedUpdateAvailable).toString()});
  });

  connect(this, &Updater::updateRequired, [this] {
    m_recommendedOrRequired = true;

    mozilla::glean::sample::update_step.record(
        mozilla::glean::sample::UpdateStepExtra{
            ._state = QVariant::fromValue(RequiredUpdateAvailable).toString()});
  });
  logger.debug() << "Updater created";
}

Updater::~Updater() {
  MZ_COUNT_DTOR(Updater);
  logger.debug() << "Updater released";
}

// static
QString Updater::appVersion() {
  if (!Constants::inProduction()) {
    return MozillaVPN::appVersionForUpdate();
  }
  return Constants::versionString();
}

// static
void Updater::updateViewShown() {
  mozilla::glean::sample::update_step.record(
      mozilla::glean::sample::UpdateStepExtra{
          ._state = QVariant::fromValue(UpdateViewShown).toString()});
}
