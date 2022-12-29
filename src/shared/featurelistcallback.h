/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURELISTCALLBACK_H
#define FEATURELISTCALLBACK_H

// Generic callback functions
// --------------------------

bool FeatureCallback_true() { return true; }

bool FeatureCallback_false() { return false; }

bool FeatureCallback_inStaging() { return !Constants::inProduction(); }

// The app must implement its feature list callback file.
#include "appfeaturelistcallback.h"

#endif  // FEATURELISTCALLBACK_H
