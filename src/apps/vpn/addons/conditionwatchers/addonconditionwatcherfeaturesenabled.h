/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONCONDITIONWATCHERFEATURESENABLED_H
#define ADDONCONDITIONWATCHERFEATURESENABLED_H

#include "addonconditionwatcher.h"

class AddonConditionWatcherFeaturesEnabled final
    : public AddonConditionWatcher {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonConditionWatcherFeaturesEnabled)

 public:
  ~AddonConditionWatcherFeaturesEnabled();

  static AddonConditionWatcher* maybeCreate(QObject* parent,
                                            const QStringList& features);

  bool conditionApplied() const override;

 private:
  AddonConditionWatcherFeaturesEnabled(QObject* parent,
                                       const QStringList& features);

 private:
  const QStringList m_features;

  bool m_currentStatus = false;
};

#endif  // ADDONCONDITIONWATCHERFEATURESENABLED_H
