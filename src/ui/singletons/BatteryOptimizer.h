/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BATTERYOPTIMIZER_H
#define BATTERYOPTIMIZER_H

#include <QObject>
#include <QQmlEngine>

#ifdef MZ_ANDROID
#  include "platforms/android/androidbatteryoptimizer.h"
#endif

class BatteryOptimizer : public QObject {
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON

  Q_PROPERTY(bool batteryOptimizationEnabled READ batteryOptimizationEnabled
                 NOTIFY stateChanged)
  Q_PROPERTY(bool canTriggerIntent READ canTriggerIntent NOTIFY stateChanged)

 public:
  bool batteryOptimizationEnabled() const {
#ifdef MZ_ANDROID
    return AndroidBatteryOptimizer::batteryOptimizationEnabled();
#else
    return false;
#endif
  }

  bool canTriggerIntent() const {
#ifdef MZ_ANDROID
    return AndroidBatteryOptimizer::canTriggerIntent();
#else
    return false;
#endif
  }

  Q_INVOKABLE void triggerBatteryOptimizationIntent() {
#ifdef MZ_ANDROID
    AndroidBatteryOptimizer::triggerBatteryOptimizationIntent();
#endif
  }

  // Call from QML after returning from the settings screen to refresh bindings.
  Q_INVOKABLE void refreshState() { emit stateChanged(); }

 signals:
  void stateChanged();
};

#endif  // BATTERYOPTIMIZER_H
