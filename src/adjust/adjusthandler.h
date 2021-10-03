/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADJUSTHELPER_H
#define ADJUSTHELPER_H

#include <QString>

class AdjustHandler {
 public:
  AdjustHandler() = default;

  static void maybeInitialize();
  static void trackEvent(const QString& event);
};

#endif  // ADJUSTHELPER_H
