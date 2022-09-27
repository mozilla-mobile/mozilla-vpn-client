/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONCONDITIONWATCHER_H
#define ADDONCONDITIONWATCHER_H

#include <QObject>

class AddonConditionWatcher : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonConditionWatcher)

 public:
  AddonConditionWatcher(QObject* parent);
  virtual ~AddonConditionWatcher();

  virtual bool conditionApplied() const = 0;

 signals:
  void conditionChanged(bool enabled);
};

#endif  // ADDONCONDITIONWATCHER_H
