/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TESTFEATURELIST_H
#define TESTFEATURELIST_H

#include "../../src/models/feature.h"

// Feature A can be turned on/off but it's always on by default
class FeatureTestA final : public Feature {
 public:
  FeatureTestA()
      : Feature("testFeatureA", "Feature A",
                false,               // Is Major Feature
                L18nStrings::Empty,  // Display name
                L18nStrings::Empty,  // Description
                L18nStrings::Empty,  // LongDescr
                "",                  // ImagePath
                "",                  // IconPath
                "",                  // link URL
                "1.0",               // released
                true,                // Can be flipped on
                true,                // Can be flipped off
                QStringList()        // feature dependencies
        ) {}

  bool checkSupportCallback() const override { return true; }

  static const FeatureTestA* instance() {
    return static_cast<const FeatureTestA*>(get("testFeatureA"));
  }
};

// Feature B can be turned on/off but it's always on by default
class FeatureTestB final : public Feature {
 public:
  FeatureTestB()
      : Feature("testFeatureB", "Feature B",
                false,               // Is Major Feature
                L18nStrings::Empty,  // Display name
                L18nStrings::Empty,  // Description
                L18nStrings::Empty,  // LongDescr
                "",                  // ImagePath
                "",                  // IconPath
                "",                  // link URL
                "1.0",               // released
                true,                // Can be flipped on
                true,                // Can be flipped off
                QStringList()        // feature dependencies
        ) {}

  bool checkSupportCallback() const override { return false; }

  static const FeatureTestB* instance() {
    return static_cast<const FeatureTestB*>(get("testFeatureB"));
  }
};

// Feature C can be turned on/off but it's always on by default
class FeatureTestC final : public Feature {
 public:
  FeatureTestC()
      : Feature("testFeatureC", "Feature C",
                false,               // Is Major Feature
                L18nStrings::Empty,  // Display name
                L18nStrings::Empty,  // Description
                L18nStrings::Empty,  // LongDescr
                "",                  // ImagePath
                "",                  // IconPath
                "",                  // link URL
                "1.0",               // released
                false,               // Can be flipped on
                false,               // Can be flipped off
                QStringList()        // feature dependencies
        ) {}

  bool checkSupportCallback() const override { return false; }

  static const FeatureTestC* instance() {
    return static_cast<const FeatureTestC*>(get("testFeatureC"));
  }
};

#endif  // TESTFEATURELIST_H
