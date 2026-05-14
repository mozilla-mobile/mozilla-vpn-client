/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidbatteryoptimizer.h"

#include <QJniObject>

#include "androidcommons.h"

constexpr auto BATTERY_HELPER_CLASS =
    "org/mozilla/firefox/qt/common/BatteryOptimizationHelper";

// static
bool AndroidBatteryOptimizer::batteryOptimizationEnabled() {
  QJniObject ctx = AndroidCommons::getActivity();
  jboolean ignoring = QJniObject::callStaticMethod<jboolean>(
      BATTERY_HELPER_CLASS, "isIgnoringBatteryOptimizations",
      "(Landroid/content/Context;)Z", ctx.object<jobject>());
  return !ignoring;
}

// static
bool AndroidBatteryOptimizer::canTriggerIntent() {
  QJniObject ctx = AndroidCommons::getActivity();
  return (bool)QJniObject::callStaticMethod<jboolean>(
      BATTERY_HELPER_CLASS, "hasRequestIgnoreBatteryOptimizationsPermission",
      "(Landroid/content/Context;)Z", ctx.object<jobject>());
}

// static
void AndroidBatteryOptimizer::triggerBatteryOptimizationIntent() {
  QJniObject ctx = AndroidCommons::getActivity();
  QJniObject intent = QJniObject::callStaticObjectMethod(
      BATTERY_HELPER_CLASS, "getRequestIgnoreBatteryOptimizationsIntent",
      "(Landroid/content/Context;)Landroid/content/Intent;",
      ctx.object<jobject>());
  if (!intent.isValid()) {
    return;
  }
  ctx.callMethod<void>("startActivity", "(Landroid/content/Intent;)V",
                       intent.object<jobject>());
}
