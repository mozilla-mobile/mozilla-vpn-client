/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IOSGLEANBRIDGE_H
#define IOSGLEANBRIDGE_H

#include "controllerimpl.h"

#include <QObject>

class IOSGleanBridge {
  Q_DISABLE_COPY_MOVE(IOSGleanBridge)

 public:
  IOSGleanBridge(bool isTelemetryEnabled, NSString* appChannel);
  ~IOSGleanBridge();
};

#endif  // IOSGLEANBRIDGE_H
