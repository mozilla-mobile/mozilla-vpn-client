/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonconditionwatcherfeaturesenabled.h"

#include "feature.h"
#include "leakdetector.h"

// static
AddonConditionWatcher* AddonConditionWatcherFeaturesEnabled::maybeCreate(
    QObject* parent, const QStringList& features) {
  if (features.isEmpty()) {
    return nullptr;
  }

  for (const QString& featureName : features) {
    const Feature* feature = Feature::getOrNull(featureName);
    if (!feature) {
      return nullptr;
    }
  }

  return new AddonConditionWatcherFeaturesEnabled(parent, features);
}

AddonConditionWatcherFeaturesEnabled::AddonConditionWatcherFeaturesEnabled(
    QObject* parent, const QStringList& features)
    : AddonConditionWatcher(parent), m_features(features) {
  MZ_COUNT_CTOR(AddonConditionWatcherFeaturesEnabled);

  m_currentStatus = conditionApplied();

  for (const QString& featureName : m_features) {
    const Feature* feature = Feature::get(featureName);
    Q_ASSERT(feature);

    connect(feature, &Feature::supportedChanged, this, [this]() {
      bool newStatus = conditionApplied();
      if (m_currentStatus != newStatus) {
        m_currentStatus = newStatus;
        emit conditionChanged(m_currentStatus);
      }
    });
  }
}

AddonConditionWatcherFeaturesEnabled::~AddonConditionWatcherFeaturesEnabled() {
  MZ_COUNT_DTOR(AddonConditionWatcherFeaturesEnabled);
}

bool AddonConditionWatcherFeaturesEnabled::conditionApplied() const {
  for (const QString& featureName : m_features) {
    // If the feature doesn't exist, we crash.
    const Feature* feature = Feature::get(featureName);
    Q_ASSERT(feature);

    if (!feature->isSupported()) {
      return false;
    }
  }

  return true;
}
