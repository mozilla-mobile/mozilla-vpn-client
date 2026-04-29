/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "featuretypes.h"

#include "settingsholder.h"

namespace Feature {

bool isEnabled(const OverridableFeature& f) {
  SettingsHolder* sh = SettingsHolder::instance();

  if (f.canFlipOn() && sh->featuresFlippedOn().contains(f.id)) {
    return true;
  }
  if (f.canFlipOff() && sh->featuresFlippedOff().contains(f.id)) {
    return false;
  }

  return f.evaluator();
}

bool isEnabled(const AnyFeature& f) {
  return std::visit(
      AnyFeatureVisitor{
          [](const ConstantFeature* s) { return s->supported; },
          [](const RuntimeFeature* d) { return d->evaluator(); },
          [](const OverridableFeature* o) { return isEnabled(*o); },
      },
      f);
}

void toggle(const OverridableFeature& f, bool enabled) {
  SettingsHolder* sh = SettingsHolder::instance();
  bool baseSupported = f.evaluator();

  QStringList on = sh->featuresFlippedOn();
  QStringList off = sh->featuresFlippedOff();

  if (enabled && !baseSupported) {
    // Off by default, force on
    if (!on.contains(f.id)) on.append(f.id);
    off.removeAll(f.id);
  } else if (!enabled && baseSupported) {
    // On by default, force off
    if (!off.contains(f.id)) off.append(f.id);
    on.removeAll(f.id);
  } else {
    // Back to default — remove any overrides
    on.removeAll(f.id);
    off.removeAll(f.id);
  }

  sh->setFeaturesFlippedOn(on);
  sh->setFeaturesFlippedOff(off);
}

}  // namespace Feature
