/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDBATTERYOPTIMIZER_H
#define ANDROIDBATTERYOPTIMIZER_H

class AndroidBatteryOptimizer {
 public:
  // true  → optimization IS enabled (VPN at risk)
  // false → app is exempted (good)
  static bool batteryOptimizationEnabled();

  // true → app holds REQUEST_IGNORE_BATTERY_OPTIMIZATIONS; system dialog shown directly
  static bool canTriggerIntent();

  // Opens battery settings (system dialog or app details depending on permission/SDK)
  static void triggerBatteryOptimizationIntent();
};

#endif  // ANDROIDBATTERYOPTIMIZER_H
