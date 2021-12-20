/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_REMOTE_RESOURCES_H
#define FEATURE_REMOTE_RESOURCES_H

#include "constants.h"
#include "models/feature.h"

constexpr const char* FEATURE_REMOTE_RESOURCES = "remoteResources";
/*
 * This Featureflag describes if the Client
 * should load remote resources (RCC) to support fast updates
 */
class FeatureRemoteResources : public Feature {
 public:
  FeatureRemoteResources()
      : Feature(FEATURE_REMOTE_RESOURCES, "Remote Resources",
                false,               // Is Major Feature
                L18nStrings::Empty,  // Display name
                L18nStrings::Empty,  // Description
                L18nStrings::Empty,  // LongDescr
                "",                  // ImagePath
                "",                  // IconPath
                "",                  // link URL
                "2.7",               // released
                true                 // Can be enabled in devmode
        ) {}

  bool checkSupportCallback() const override { return false; }

  static const FeatureRemoteResources* instance() {
    return static_cast<const FeatureRemoteResources*>(
        get(FEATURE_REMOTE_RESOURCES));
  }
};

#endif  // FEATURE_REMOTE_RESOURCES_H
