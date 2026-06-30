/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONCONDITIONWATCHERLOCALES_H
#define ADDONCONDITIONWATCHERLOCALES_H

#include "addonconditionwatcher.h"

class AddonConditionWatcherLocales final : public AddonConditionWatcher {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonConditionWatcherLocales)

 public:
  ~AddonConditionWatcherLocales();

  static AddonConditionWatcher* maybeCreate(QObject* parent,
                                            const QStringList& locales);

  bool conditionApplied() const override;

 private:
  AddonConditionWatcherLocales(QObject* parent, const QStringList& locales);

 private:
  const QStringList m_locales;

  bool m_currentStatus = false;
};

#endif  // ADDONCONDITIONWATCHERLOCALES_H
