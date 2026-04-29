/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonconditionwatcherfeaturesenabled.h"

#include "feature/featuremodel.h"
#include "feature/featureproxy.h"
#include "leakdetector.h"

// static
AddonConditionWatcher* AddonConditionWatcherFeaturesEnabled::maybeCreate(
    QObject* parent, const QStringList& features) {
  if (features.isEmpty()) {
    return nullptr;
  }

  for (const QString& featureName : features) {
    if (!FeatureModel::instance()->get(featureName)) {
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
    auto* proxy =
        qobject_cast<FeatureProxy*>(FeatureModel::instance()->get(featureName));
    Q_ASSERT(proxy);

    connect(proxy, &FeatureProxy::supportedChanged, this, [this]() {
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
    if (!FeatureModel::instance()->isEnabledById(featureName)) {
      return false;
    }
  }

  return true;
}
