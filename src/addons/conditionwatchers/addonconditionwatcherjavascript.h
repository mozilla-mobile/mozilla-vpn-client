/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONCONDITIONWATCHERJAVASCRIPT_H
#define ADDONCONDITIONWATCHERJAVASCRIPT_H

#include <QJSValue>

#include "addonconditionwatcher.h"

class Addon;

class AddonConditionWatcherJavascript final : public AddonConditionWatcher {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonConditionWatcherJavascript)

 public:
  ~AddonConditionWatcherJavascript();

  Q_INVOKABLE void enable();
  Q_INVOKABLE void disable();

  static AddonConditionWatcher* maybeCreate(Addon* addon,
                                            const QString& javascript);

  bool conditionApplied() const override;

 private:
  AddonConditionWatcherJavascript(Addon* addon, const QJSValue& function);

 private:
  bool m_currentStatus = false;
};

#endif  // ADDONCONDITIONWATCHERJAVASCRIPT_H
