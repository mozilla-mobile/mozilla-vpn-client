/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONCONDITIONWATCHERTRANSLATIONTHRESHOLD_H
#define ADDONCONDITIONWATCHERTRANSLATIONTHRESHOLD_H

class Addon;
class AddonConditionWatcher;

class AddonConditionWatcherTranslationThreshold final {
 public:
  static AddonConditionWatcher* maybeCreate(Addon* addon,
                                            double translationThreshold);
};

#endif  // ADDONCONDITIONWATCHERTRANSLATIONTHRESHOLD_H
