/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IOSGLEANGLUE_H
#define IOSGLEANGLUE_H

#include "controllerimpl.h"

#include <QObject>

class IOSGleanGlue {
  Q_DISABLE_COPY_MOVE(IOSGleanGlue)

 public:
  IOSGleanGlue(bool isTelemetryEnabled);
  ~IOSGleanGlue();
};

#endif  // IOSGLEANGLUE_H
