/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_WEBSOCKET_H
#define FEATURE_WEBSOCKET_H

#include "models/feature.h"

constexpr const char* FEATURE_WEBSOCKET = "websocket";

class FeatureWebSocket : public Feature {
 public:
  FeatureWebSocket()
      : Feature(FEATURE_WEBSOCKET, "WebSocket",
                false,               // Is Major Feature
                L18nStrings::Empty,  // Display name
                L18nStrings::Empty,  // Description
                L18nStrings::Empty,  // LongDescr
                "",                  // ImagePath
                "",                  // IconPath
                "",                  // link URL
                "2.9.0",             // released
                true,                // Can be flipped on
                true,                // Can be flipped off
                QStringList()        // feature dependencies
        ) {}

  bool checkSupportCallback() const override { return false; }

  static const FeatureWebSocket* instance() {
    return static_cast<const FeatureWebSocket*>(get(FEATURE_WEBSOCKET));
  }
};

#endif  // FEATURE_WEBSOCKET_H
